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

    /// @brief Turn value back to double.
    /// @param aInteger
    /// @return const double
    inline const double toDouble(const int aInteger) const { return (double)aInteger / MULTIPLIER; };

    const Json::Value itemToJson(const std::shared_ptr<Item> item) const
    {
        Json::Value JsonItem;
        JsonItem[constants::json::item::ID] = item->Item::id_;
        JsonItem[constants::json::item::WEIGHT] = item->getRealWeight();
        JsonItem[constants::json::item::VOLUME] = item->getRealVolume();
        JsonItem[constants::json::item::ALLOWED_ROTATIONS] = item->Item::allowedRotations_;

        JsonItem[constants::json::item::ROTATION_TYPE] = item->Item::rotationType_;
        JsonItem[constants::json::item::ROTATION_TYPE_DESCRIPTION] = item->Item::rotationTypeDescription_;

        JsonItem[constants::json::item::WIDTH] = (ResponseBuilder::itemDimensionsAfter_ ? item->getRealWidth() : item->getRealOriginalWidth());
        JsonItem[constants::json::item::DEPTH] = (ResponseBuilder::itemDimensionsAfter_ ? item->getRealDepth() : item->getRealOriginalDepth());
        JsonItem[constants::json::item::HEIGHT] = (ResponseBuilder::itemDimensionsAfter_ ? item->getRealHeight() : item->getRealOriginalHeight());
        JsonItem[constants::json::item::X_COORDINATE] = item->getRealXPosition();
        JsonItem[constants::json::item::Y_COORDINATE] = item->getRealYPosition();
        JsonItem[constants::json::item::Z_COORDINATE] = item->getRealZPosition();

        if (ResponseBuilder::hasValue(item->Item::itemConsolidationKey_))
        {
            JsonItem[constants::json::item::ITEM_CONS_KEY] = item->Item::itemConsolidationKey_;
        };

        // Stacking style.
        if (item->Item::stackingStyle_ == constants::item::parameter::ALLOW_ALL)
        {
            JsonItem[constants::json::item::STACKING_STYLE] = constants::json::item::stackingStyle::ALLOW_ALL;
        }
        else if (item->Item::stackingStyle_ == constants::item::parameter::NO_ITEMS_ON_TOP)
        {
            JsonItem[constants::json::item::STACKING_STYLE] = constants::json::item::stackingStyle::NO_ITEMS_ON_TOP;
        }
        else if (item->Item::stackingStyle_ == constants::item::parameter::BOTTOM_NO_ITEMS_ON_TOP)
        {
            JsonItem[constants::json::item::STACKING_STYLE] = constants::json::item::stackingStyle::BOTTOM_NO_ITEMS_ON_TOP;
        }

        return JsonItem;
    };

    const Json::Value binToJson(const std::shared_ptr<Bin> bin) const
    {
        Json::Value mappedBin;
        mappedBin[constants::json::outbound::bin::ID] = bin->id_;
        mappedBin[constants::json::outbound::bin::TYPE] = bin->type_;
        mappedBin[constants::json::outbound::bin::NR_OF_ITEMS] = int(bin->getFittedItems().size());
        mappedBin[constants::json::outbound::bin::WIDTH] = bin->getRealWidth();
        mappedBin[constants::json::outbound::bin::DEPTH] = bin->getRealDepth();
        mappedBin[constants::json::outbound::bin::HEIGHT] = bin->getRealHeight();

        mappedBin[constants::json::outbound::bin::MAX_VOLUME] = bin->volume_;
        mappedBin[constants::json::outbound::bin::ACTUAL_VOLUME] = bin->getRealActualVolumeUtil();
        mappedBin[constants::json::outbound::bin::ACTUAL_VOLUME_UTIL] = bin->getRealActualVolumeUtilPercentage();

        mappedBin[constants::json::outbound::bin::MAX_WEIGHT] = bin->getRealMaxWeight();
        mappedBin[constants::json::outbound::bin::ACTUAL_WEIGHT] = bin->getRealActualWeightUtil();
        mappedBin[constants::json::outbound::bin::ACTUAL_WEIGHT_UTIL] = bin->getRealActualWeightUtilPercentage();

        mappedBin[constants::json::outbound::bin::FURTHEST_POINT_WIDTH] = bin->getRealFurthestPointWidth();
        mappedBin[constants::json::outbound::bin::FURTHEST_POINT_DEPTH] = bin->getRealFurthestPointDepth();
        mappedBin[constants::json::outbound::bin::FURTHEST_POINT_HEIGHT] = bin->getRealFurthestPointHeight();

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
     * It takes a binComposer as input and converts its content to the outgoing json.
     *
     * @param aBinComposer
     */
    void generate(std::shared_ptr<BinComposer> aBinComposer)
    {

        /*
        Aggregate unfitted items across packing clusters.
        If there are unfitted items, create unfitted items json section.
        */

        std::vector<int> myUnfittedItems;
        myUnfittedItems.insert(myUnfittedItems.end(), aBinComposer->getItemsToBePacked().begin(),
                               aBinComposer->getItemsToBePacked().end());

        if (!myUnfittedItems.empty())
        {
            outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS] = Json::arrayValue;
            for (const int &it : myUnfittedItems)
            {
                outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS].append(
                    ResponseBuilder::itemToJson(
                        aBinComposer->getMasterItemRegister()->getItem(it)));
            };
        };

        /* Return if no bins were packed. Happens if none of the items fit. */
        if (aBinComposer->getNumberOfBins() == 0)
        {
            return;
        }

        /* Header information. */
        outboundRoot_[constants::json::outbound::header::REQUIRED_NR_OF_BINS] = aBinComposer->getNumberOfBins();
        outboundRoot_[constants::json::outbound::header::TOTAL_VOLUME_UTIL] = aBinComposer->getTotalVolumeUtilPercentage();
        outboundRoot_[constants::json::outbound::header::TOTAL_WEIGHT_UTIL] = aBinComposer->getTotalWeightUtilPercentage();

        /* If includeBins parameter is false, skip generating json for the bins. */
        if (!ResponseBuilder::includeBins_)
        {
            return;
        };

        for (const std::shared_ptr<Bin> &bin : aBinComposer->getPackedBins())
        {

            Json::Value mappedBin = ResponseBuilder::binToJson(bin);

            /* If includeItems parameter is false, skip generating json for the items. */
            if (ResponseBuilder::includeItems_)
            {
                for (const auto &item : bin->Bin::getFittedItems())
                {
                    mappedBin[constants::json::outbound::bin::FITTED_ITEMS].append(
                        ResponseBuilder::itemToJson(bin->getContext()->getItem(item)));
                };
            };

            ResponseBuilder::outboundRoot_[constants::json::outbound::PACKED_BINS].append(mappedBin);
        };
    };
};

#endif