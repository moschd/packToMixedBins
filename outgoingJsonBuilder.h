#ifndef RESPONSE_BUILDER_H
#define RESPONSE_BUILDER_H

class ResponseBuilder
{
private:
    int precision_;
    std::string indentation_;
    bool includeItems_;
    bool includeBins_;
    bool itemDimensionsAfter_;
    Json::StreamWriterBuilder builder_;
    Json::Value outboundRoot_;

    void configureBuilder()
    {
        ResponseBuilder::builder_["indentation"] = ResponseBuilder::indentation_;
        ResponseBuilder::builder_.settings_["precision"] = ResponseBuilder::precision_;
    };

    const Json::Value itemToJson(const Item &item) const
    {
        Json::Value JsonItem;
        JsonItem[constants::json::item::ID] = item.Item::id_;
        JsonItem[constants::json::item::WIDTH] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::width_ : item.Item::original_width_);
        JsonItem[constants::json::item::DEPTH] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::depth_ : item.Item::original_depth_);
        JsonItem[constants::json::item::HEIGHT] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::height_ : item.Item::original_height_);
        JsonItem[constants::json::item::WEIGHT] = item.Item::weight_;
        JsonItem[constants::json::item::VOLUME] = item.Item::maxVolume_;

        if (item.Item::itemConsolidationKey_.size())
        {
            JsonItem[constants::json::item::ITEM_CONS_KEY] = item.Item::itemConsolidationKey_;
        };

        JsonItem[constants::json::item::ALLOWED_ROTATIONS] = item.Item::allowedRotations_;
        JsonItem[constants::json::item::X_COORDINATE] = item.Item::position_[constants::axis::WIDTH];
        JsonItem[constants::json::item::Y_COORDINATE] = item.Item::position_[constants::axis::DEPTH];
        JsonItem[constants::json::item::Z_COORDINATE] = item.Item::position_[constants::axis::HEIGHT];

        if (0 <= item.Item::rotationType_ && item.Item::rotationType_ < 6)
        {
            JsonItem[constants::json::item::ROTATION_TYPE] = item.Item::rotationType_;
        };

        if (item.Item::rotationTypeDescription_.size())
        {
            JsonItem[constants::json::item::ROTATION_TYPE_DESCRIPTION] = item.Item::rotationTypeDescription_;
        };

        return JsonItem;
    };

    const Json::Value binToJson(const Bin &bin) const
    {
        Json::Value mappedBin;
        mappedBin[constants::json::bin::ID] = int(bin.Bin::id_);
        mappedBin[constants::json::bin::TYPE] = bin.Bin::type_;
        mappedBin[constants::json::bin::NR_OF_ITEMS] = int(bin.Bin::getFittedItems().size());

        mappedBin[constants::json::bin::MAX_WIDTH] = bin.Bin::width_;
        mappedBin[constants::json::bin::MAX_DEPTH] = bin.Bin::depth_;
        mappedBin[constants::json::bin::MAX_HEIGHT] = bin.Bin::height_;

        mappedBin[constants::json::bin::MAX_VOLUME] = bin.Bin::maxVolume_;
        mappedBin[constants::json::bin::ACTUAL_VOLUME] = bin.Bin::getActVolumeUtil();
        mappedBin[constants::json::bin::ACTUAL_VOLUME_UTIL] = bin.Bin::getActVolumeUtilizationPercentage();

        mappedBin[constants::json::bin::MAX_WEIGHT] = bin.Bin::maxWeight_;
        mappedBin[constants::json::bin::ACTUAL_WEIGHT] = bin.Bin::getActWeightUtil();
        mappedBin[constants::json::bin::ACTUAL_WEIGHT_UTIL] = bin.Bin::getActWeightUtilizationPercentage();

        return mappedBin;
    };

    void exceptionJson(const int exceptionType)
    {
        switch (exceptionType)
        {
        case 10:
            ResponseBuilder::outboundRoot_[constants::json::outbound::EXCEPTION] = "Every single item exceeds the bin capacity. None of the items could be packed.";
            break;
        };
    }

public:
    const Json::StreamWriterBuilder &getBuilder() const { return ResponseBuilder::builder_; };

    const Json::Value &getMessage() const { return ResponseBuilder::outboundRoot_; };

    ResponseBuilder(int aPrecision, bool aIncludeBins, bool aIncludeItems, bool aItemDimensionsAfter) : indentation_(""),
                                                                                                        precision_(aPrecision),
                                                                                                        includeBins_(aIncludeBins),
                                                                                                        includeItems_(aIncludeItems),
                                                                                                        itemDimensionsAfter_(aItemDimensionsAfter)
    {
        ResponseBuilder::configureBuilder();
    };

    /**
     * @brief Generates the outgoing JSON.
     *
     * It takes a packed packing processor as input and converts its content to the outgoing json.
     * Generate a exception json incase none of the provided items are packed into a bin.
     *
     * @param PackingProcessor
     */
    void generate(Packer PackingProcessor)
    {

        if (!PackingProcessor.Packer::getPackedBinVector().size())
        {
            ResponseBuilder::exceptionJson(10);
            return;
        };

        /* Header information. */
        outboundRoot_[constants::json::outbound::header::REQUIRED_NR_OF_BINS] = int(PackingProcessor.Packer::getPackedBinVector().size());
        outboundRoot_[constants::json::outbound::header::TOTAL_VOLUME_UTIL] = PackingProcessor.Packer::GetTotalVolumeUtilizationPercentage();
        outboundRoot_[constants::json::outbound::header::TOTAL_WEIGHT_UTIL] = PackingProcessor.Packer::GetTotalWeightUtilizationPercentage();

        /* Check for items that did not get packed and include them in the header. */
        if (!PackingProcessor.Packer::GetLastBin().Bin::getUnfittedItems().empty())
        {
            outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS] = Json::arrayValue;
            for (auto &it : PackingProcessor.Packer::GetLastBin().Bin::getUnfittedItems())
            {
                outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS].append(
                    ResponseBuilder::itemToJson(
                        PackingProcessor.Packer::masterItemRegister_->ItemRegister::getConstItem(it)));
            };
        };

        for (auto &bi : PackingProcessor.Packer::getPackedBinVector())
        {
            /* Free memory again, how to do this in a more structured way? */
            bi.Bin::kdTree_->KdTree::deleteAllNodesHelper();
            delete bi.Bin::kdTree_;
        }

        if (!ResponseBuilder::includeBins_)
        {
            return;
        };

        for (const auto &bin : PackingProcessor.Packer::getPackedBinVector())
        {
            Json::Value mappedBin = ResponseBuilder::binToJson(bin);

            if (ResponseBuilder::includeItems_)
            {
                for (const auto &item : bin.Bin::getFittedItems())
                {
                    mappedBin[constants::json::bin::FITTED_ITEMS].append(
                        ResponseBuilder::itemToJson(PackingProcessor.Packer::masterItemRegister_->ItemRegister::getConstItem(item)));
                };
            };
            ResponseBuilder::outboundRoot_[constants::json::outbound::PACKED_BINS].append(mappedBin);
        };
    };
};

#endif