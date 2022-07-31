#ifndef PACKER_H
#define PACKER_H
class Packer
{
private:
    std::vector<Bin> bins_;
    int estimatedTotalRequiredBins_;

public:
    std::string requestedBinType_;
    double requestedBinWidth_;
    double requestedBinDepth_;
    double requestedBinHeight_;
    double requestedBinMaxWeight_;
    double requestedBinMaxVolume_;
    ItemRegister *masterItemRegister_;
    Gravity *masterGravity_;

    Packer(std::string aBinType, double aBinWidth, double aBinDepth, double aBinHeight, double aBinMaxWeight,
           Gravity &aGravity, ItemRegister &aItemRegister)
    {
        requestedBinType_ = aBinType;
        requestedBinWidth_ = aBinWidth;
        requestedBinDepth_ = aBinDepth;
        requestedBinHeight_ = aBinHeight;
        requestedBinMaxWeight_ = aBinMaxWeight;
        requestedBinMaxVolume_ = (aBinWidth * aBinHeight * aBinDepth);
        masterGravity_ = &aGravity;
        masterItemRegister_ = &aItemRegister;
    };

    Bin &GetLastBin()
    {
        return bins_.back();
    };
    std::vector<Bin> &GetPackedBinVector()
    {
        return bins_;
    };

    double GetTotalVolumeUtilizationPercentage()
    {
        double actualVolumeUtil = 0;
        for (auto &b : bins_)
        {
            actualVolumeUtil += b.GetActVolumeUtil();
        };
        return std::max(0.0, actualVolumeUtil / (requestedBinWidth_ * requestedBinDepth_ * requestedBinHeight_ * bins_.size()) * 100);
    };

    double GetTotalWeightUtilizationPercentage()
    {
        double actualWeightUtil = 0;
        for (auto &b : bins_)
        {
            actualWeightUtil += b.GetActWeightUtil();
        };
        return std::max(0.0, actualWeightUtil / (requestedBinMaxWeight_ * bins_.size()) * 100);
    };

    /**
     * @brief Return an integer representing the estimated number of items that will fit a empty bin.
     *
     * This method iterates over the itemKeys vector and makes an estimation of how many of these items will fit into an empty bin.
     * Estimation is fully based on volume.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    int estimatedNumberOfItemsThatWillFitIntoBin(std::vector<int> &aItemsToBePacked)
    {
        int estimatedNumberOfItems = 0;
        double cumulativeItemVolume = 0.0;
        for (auto itemKey : aItemsToBePacked)
        {
            if ((masterItemRegister_->getItem(itemKey).volume_ + cumulativeItemVolume) < requestedBinMaxVolume_)
            {
                estimatedNumberOfItems += 1;
                cumulativeItemVolume += masterItemRegister_->getItem(itemKey).volume_;
            };
        };
        return estimatedNumberOfItems;
    };

    /**
     * @brief Start to add item(s) into a bin.
     *
     * This method iterates over the item vector and tries to place each item into the bin. If a bin is full it
     * creates a new bin and the process starts over, now the input is the previous bin' unfitted items.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    void startPacking(std::vector<int> aItemsToBePacked)
    {
        if (aItemsToBePacked.empty())
        {
            return;
        };

        bins_.push_back(Bin(requestedBinType_,
                            (bins_.size() + 1),
                            requestedBinWidth_,
                            requestedBinDepth_,
                            requestedBinHeight_,
                            requestedBinMaxWeight_,
                            masterGravity_,
                            masterItemRegister_,
                            estimatedNumberOfItemsThatWillFitIntoBin(aItemsToBePacked)));

        for (auto &item_to_pack : aItemsToBePacked)
        {
            Item *itp = &masterItemRegister_->getItem(item_to_pack);

            /* Check if item would exceed weight or volume limit. */
            if ((bins_.back().GetActVolumeUtil() + itp->volume_) > bins_.back().maxVolume_ ||
                (bins_.back().GetActWeightUtil() + itp->weight_) > bins_.back().maxWeight_)
            {
                bins_.back().GetUnfittedItems().push_back(itp->transientSysId_);
                continue;
            };

            /* Check if item would be the first item in the bin, if so take shortcut. */
            if (bins_.back().GetFittedItems().empty())
            {
                if (bins_.back().PlaceItemInBin(item_to_pack))
                {
                    bins_.back().UpdateWithNewFittedItem(item_to_pack, 0);
                    continue;
                };
            };

            /* Check if item is the same as previous unfitted item. */
            if (!bins_.back().GetUnfittedItems().empty() &&
                masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).width_ == itp->width_ &&
                masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).height_ == itp->height_ &&
                masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).depth_ == itp->depth_)
            {
                bins_.back().GetUnfittedItems().push_back(itp->transientSysId_);
                continue;
            };

            bins_.back().FindItemPosition(item_to_pack);
        };

        /* Delete the created bin if it contains no items. */
        if (aItemsToBePacked.size() == bins_.back().GetUnfittedItems().size())
        {
            GetPackedBinVector().pop_back();
            return;
        };

        startPacking(bins_.back().GetUnfittedItems());
    };
};

#endif