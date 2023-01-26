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

    inline const bool hasValue(std::string aString) const
    {
        return aString.size();
    };

    const Json::Value itemToJson(const Item &item) const
    {
        Json::Value JsonItem;
        JsonItem[constants::json::item::ID] = item.Item::id_;
        JsonItem[constants::json::item::HEIGHT] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::height_ : item.Item::original_height_);
        JsonItem[constants::json::item::WEIGHT] = item.Item::weight_;
        JsonItem[constants::json::item::VOLUME] = item.Item::volume_;
        JsonItem[constants::json::item::ALLOWED_ROTATIONS] = item.Item::allowedRotations_;

        JsonItem[constants::json::item::ROTATION_TYPE] = item.Item::rotationType_;
        JsonItem[constants::json::item::ROTATION_TYPE_DESCRIPTION] = item.Item::rotationTypeDescription_;

        JsonItem[constants::json::item::WIDTH] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::width_ : item.Item::original_width_);
        JsonItem[constants::json::item::DEPTH] = (ResponseBuilder::itemDimensionsAfter_ ? item.Item::depth_ : item.Item::original_depth_);
        JsonItem[constants::json::item::X_COORDINATE] = item.Item::position_[constants::axis::WIDTH];
        JsonItem[constants::json::item::Y_COORDINATE] = item.Item::position_[constants::axis::DEPTH];
        JsonItem[constants::json::item::Z_COORDINATE] = item.Item::position_[constants::axis::HEIGHT];

        if (ResponseBuilder::hasValue(item.Item::itemConsolidationKey_))
        {
            JsonItem[constants::json::item::ITEM_CONS_KEY] = item.Item::itemConsolidationKey_;
        };
        return JsonItem;
    };

    const Json::Value binToJson(const Bin &bin) const
    {
        Json::Value mappedBin;
        mappedBin[constants::json::outbound::bin::ID] = bin.Bin::id_;
        mappedBin[constants::json::outbound::bin::TYPE] = bin.Bin::type_;
        mappedBin[constants::json::outbound::bin::NR_OF_ITEMS] = int(bin.Bin::getFittedItems().size());
        mappedBin[constants::json::outbound::bin::MAX_WIDTH] = bin.Bin::width_;
        mappedBin[constants::json::outbound::bin::MAX_DEPTH] = bin.Bin::depth_;
        mappedBin[constants::json::outbound::bin::MAX_HEIGHT] = bin.Bin::height_;
        mappedBin[constants::json::outbound::bin::MAX_VOLUME] = bin.Bin::volume_;
        mappedBin[constants::json::outbound::bin::ACTUAL_VOLUME] = bin.Bin::getActVolumeUtil();
        mappedBin[constants::json::outbound::bin::ACTUAL_VOLUME_UTIL] = bin.Bin::getActVolumeUtilPercentage();
        mappedBin[constants::json::outbound::bin::MAX_WEIGHT] = bin.Bin::maxWeight_;
        mappedBin[constants::json::outbound::bin::ACTUAL_WEIGHT] = bin.Bin::getActWeightUtil();
        mappedBin[constants::json::outbound::bin::ACTUAL_WEIGHT_UTIL] = bin.Bin::getActWeightUtilPercentage();

        return mappedBin;
    };

public:
    const Json::StreamWriterBuilder &getBuilder() const { return ResponseBuilder::builder_; };

    const Json::Value &getMessage() const { return ResponseBuilder::outboundRoot_; };

    ResponseBuilder(int aPrecision,
                    bool aIncludeBins,
                    bool aIncludeItems,
                    bool aItemDimensionsAfter) : indentation_(""),
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
     *
     * @param packedPacker
     */
    void generate(Packer packedPacker)
    {

        /*
        Aggregate unfitted items across packing clusters.
        If there are unfitted items, create unfitted items json section.
        */

        std::vector<int> myUnfittedItems;
        for (const auto &cluster : packedPacker.getClusters())
        {
            myUnfittedItems.insert(myUnfittedItems.end(), cluster.getUnfittedItems().begin(),
                                   cluster.getUnfittedItems().end());
        };

        if (!myUnfittedItems.empty())
        {
            outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS] = Json::arrayValue;
            for (const int &it : myUnfittedItems)
            {
                outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS].append(
                    ResponseBuilder::itemToJson(
                        packedPacker.Packer::getContext()->getItem(it)));
            };
        };

        /* Return if no bins were packed. Happens if none of the items fit. */
        if (packedPacker.Packer::getNumberOfBins() == 0)
        {
            return;
        }

        /* Header information. */
        outboundRoot_[constants::json::outbound::header::REQUIRED_NR_OF_BINS] = packedPacker.Packer::getNumberOfBins();
        outboundRoot_[constants::json::outbound::header::TOTAL_VOLUME_UTIL] = packedPacker.Packer::getTotalVolumeUtilPercentage();
        outboundRoot_[constants::json::outbound::header::TOTAL_WEIGHT_UTIL] = packedPacker.Packer::getTotalWeightUtilPercentage();

        /* Free allocated memory.
        TODO Find a way to do this in a correct way, look for best practices. */
        packedPacker.freeMemory();

        /* If includeBins parameter is false, skip generating json for the bins. */
        if (!ResponseBuilder::includeBins_)
        {
            return;
        };

        for (const auto &cluster : packedPacker.Packer::getClusters())
        {
            for (const auto &bin : cluster.PackingCluster::getPackedBins())
            {

                Json::Value mappedBin = ResponseBuilder::binToJson(bin);

                /* If includeItems parameter is false, skip generating json for the items. */
                if (ResponseBuilder::includeItems_)
                {
                    for (const auto &item : bin.Bin::getFittedItems())
                    {
                        mappedBin[constants::json::outbound::bin::FITTED_ITEMS].append(
                            ResponseBuilder::itemToJson(packedPacker.Packer::getContext()->getItem(item)));
                    };
                };

                ResponseBuilder::outboundRoot_[constants::json::outbound::PACKED_BINS].append(mappedBin);
            };
        };
    };
};

#endif