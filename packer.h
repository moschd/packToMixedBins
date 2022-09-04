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

    Packer(std::string aBinType,
           double aBinWidth,
           double aBinDepth,
           double aBinHeight,
           double aBinMaxWeight,
           Gravity &aGravity,
           ItemRegister &aItemRegister) : requestedBinType_(aBinType),
                                          requestedBinWidth_(aBinWidth),
                                          requestedBinDepth_(aBinDepth),
                                          requestedBinHeight_(aBinHeight),
                                          requestedBinMaxWeight_(aBinMaxWeight),
                                          masterGravity_(&aGravity),
                                          masterItemRegister_(&aItemRegister)

    {
        Packer::requestedBinMaxVolume_ = (requestedBinWidth_ * requestedBinDepth_ * requestedBinHeight_);
    };

    const Bin &GetLastBin() const
    {
        return Packer::bins_.back();
    };
    const std::vector<Bin> &getPackedBinVector() const
    {
        return Packer::bins_;
    };

    void deleteLastBin()
    {
        Packer::bins_.pop_back();
    };

    double GetTotalVolumeUtilizationPercentage()
    {
        double actualVolumeUtil = 0;
        for (auto &b : Packer::bins_)
        {
            actualVolumeUtil += b.Bin::getActVolumeUtil();
        };
        return std::max(0.0, actualVolumeUtil / (Packer::requestedBinWidth_ * Packer::requestedBinDepth_ * Packer::requestedBinHeight_ * Packer::bins_.size()) * 100);
    };

    double GetTotalWeightUtilizationPercentage()
    {
        double actualWeightUtil = 0;
        for (auto &b : Packer::bins_)
        {
            actualWeightUtil += b.Bin::getActWeightUtil();
        };
        return std::max(0.0, actualWeightUtil / (Packer::requestedBinMaxWeight_ * Packer::bins_.size()) * 100);
    };

    void addUnfittedItem(int itemKey)
    {
        Packer::bins_.back().Bin::addUnfittedItem(itemKey);
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
            if ((Packer::masterItemRegister_->ItemRegister::getItem(itemKey).Item::maxVolume_ + cumulativeItemVolume) < Packer::requestedBinMaxVolume_)
            {
                estimatedNumberOfItems += 1;
                cumulativeItemVolume += Packer::masterItemRegister_->ItemRegister::getItem(itemKey).Item::maxVolume_;
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

        Packer::bins_.push_back(Bin(Packer::requestedBinType_,
                                    (Packer::bins_.size() + 1),
                                    Packer::requestedBinWidth_,
                                    Packer::requestedBinDepth_,
                                    Packer::requestedBinHeight_,
                                    Packer::requestedBinMaxWeight_,
                                    Packer::masterGravity_,
                                    Packer::masterItemRegister_,
                                    Packer::estimatedNumberOfItemsThatWillFitIntoBin(aItemsToBePacked)));

        for (auto &item_to_pack : aItemsToBePacked)
        {
            Item *itp = &Packer::masterItemRegister_->ItemRegister::getItem(item_to_pack);

            /* Check if item would exceed weight or volume limit. */
            if ((Packer::bins_.back().Bin::getActVolumeUtil() + itp->Item::maxVolume_) > Packer::bins_.back().Bin::maxVolume_ ||
                (Packer::bins_.back().Bin::getActWeightUtil() + itp->Item::weight_) > Packer::bins_.back().Bin::maxWeight_)
            {
                Packer::addUnfittedItem(itp->Item::transientSysId_);
                continue;
            };

            /* Check if item would be the first item in the bin, if so take shortcut. */
            if (Packer::bins_.back().Bin::getFittedItems().empty())
            {
                if (Packer::bins_.back().Bin::placeItemInBin(item_to_pack))
                {
                    Packer::bins_.back().Bin::updateWithNewFittedItem(item_to_pack, 0);
                    continue;
                };
            };

            /* Check if item is the same as previous unfitted item. */
            if (!Packer::bins_.back().Bin::getUnfittedItems().empty() &&
                Packer::masterItemRegister_->ItemRegister::getItem(Packer::bins_.back().Bin::getUnfittedItems().back()).Item::width_ == itp->Item::width_ &&
                Packer::masterItemRegister_->ItemRegister::getItem(Packer::bins_.back().Bin::getUnfittedItems().back()).Item::height_ == itp->Item::height_ &&
                Packer::masterItemRegister_->ItemRegister::getItem(Packer::bins_.back().Bin::getUnfittedItems().back()).Item::depth_ == itp->Item::depth_)
            {
                Packer::addUnfittedItem(itp->Item::transientSysId_);
                continue;
            };

            Packer::bins_.back().Bin::findItemPosition(item_to_pack);
        };

        /* Delete the created bin if it contains no items. */
        if (aItemsToBePacked.size() == Packer::bins_.back().Bin::getUnfittedItems().size())
        {
            Packer::deleteLastBin();
            return;
        };

        Packer::startPacking(bins_.back().Bin::getUnfittedItems());
    };
};

#endif