#ifndef PACKING_CLUSER_H
#define PACKING_CLUSER_H

class PackingCluster
{
private:
    std::vector<Bin> bins_;
    int estimatedTotalRequiredBins_;
    std::string requestedBinType_;
    double requestedBinWidth_;
    double requestedBinDepth_;
    double requestedBinHeight_;
    double requestedBinMaxWeight_;
    double requestedBinMaxVolume_;
    Gravity *masterGravity_;

    /**
     * @brief Checks if the item is the same as the most recent unfitted item.
     *
     * Return false if no item is unfitted yet.
     * Return true if items are of the same shape, have the same dimensions, and have the same allowed rotations.
     *
     * @param aItemToCheck
     * @return true
     * @return false
     */
    const bool equalsPreviousUnfittedItem(const Item *aItemToCheck) const
    {
        bool isTheSame = 0;

        if (PackingCluster::bins_.back().Bin::getUnfittedItems().empty())
        {
            return isTheSame;
        };

        const Item *lastUnfittedItem = &PackingCluster::masterItemRegister_->ItemRegister::getConstItem(PackingCluster::bins_.back().Bin::getUnfittedItems().back());

        if (lastUnfittedItem->isShape(constants::shape::CUBOID) && aItemToCheck->isShape(constants::shape::CUBOID) &&
            lastUnfittedItem->Item::width_ == aItemToCheck->Item::width_ &&
            lastUnfittedItem->Item::depth_ == aItemToCheck->Item::depth_ &&
            lastUnfittedItem->Item::height_ == aItemToCheck->Item::height_ &&
            lastUnfittedItem->Item::allowedRotations_ == aItemToCheck->Item::allowedRotations_)
        {
            isTheSame = 1;
        }
        else if (lastUnfittedItem->isShape(constants::shape::CYLINDER) && aItemToCheck->isShape(constants::shape::CYLINDER) &&
                 lastUnfittedItem->Item::diameter_ == aItemToCheck->Item::diameter_ &&
                 lastUnfittedItem->Item::height_ == aItemToCheck->Item::height_ &&
                 lastUnfittedItem->Item::allowedRotations_ == aItemToCheck->Item::allowedRotations_)
        {
            isTheSame = 1;
        };

        return isTheSame;
    };

public:
    int id_;
    std::string consolidationId_;
    ItemRegister *masterItemRegister_;

    PackingCluster(unsigned int aId,
                   std::string aConsolidationId,
                   std::string aBinType,
                   double aBinWidth,
                   double aBinDepth,
                   double aBinHeight,
                   double aBinMaxWeight,
                   double aBinMaxVolume,
                   Gravity &aGravity,
                   ItemRegister &aItemRegister) : id_(aId),
                                                  consolidationId_(aConsolidationId),
                                                  requestedBinType_(aBinType),
                                                  requestedBinWidth_(aBinWidth),
                                                  requestedBinDepth_(aBinDepth),
                                                  requestedBinHeight_(aBinHeight),
                                                  requestedBinMaxWeight_(aBinMaxWeight),
                                                  requestedBinMaxVolume_(aBinMaxVolume),
                                                  masterGravity_(&aGravity),
                                                  masterItemRegister_(&aItemRegister){};

    /**
     * @brief Get the last created bin object based on Id_.
     *
     * @return const Bin&
     */
    const Bin &getBinById(const int binToGet) const
    {
        for (auto &bin : PackingCluster::bins_)
        {
            if (bin.id_ == binToGet)
            {
                return bin;
            }
        };

        /* default, should never happen. */
        return PackingCluster::bins_.back();
    };

    /**
     * @brief Get all packed bins.
     *
     * @return const std::vector<Bin>&
     */
    const std::vector<Bin> &getPackedBins() const
    {
        return PackingCluster::bins_;
    };

    /**
     * @brief Get all last bin.
     *
     * @return const std::vector<Bin>&
     */
    const std::vector<Bin> &getLastCreatedBin() const
    {
        const int numberOfBins = PackingCluster::bins_.size();

        return PackingCluster::bins_;
    };

    /**
     * @brief Remove the last bin.
     *
     */
    void deleteLastBin()
    {
        PackingCluster::bins_.pop_back();
    };

    /**
     * @brief Set all bins.
     *
     * Used when updating bins that have been processed by the distributor.
     *
     * @return double
     */
    void setBins(std::vector<Bin> aBins)
    {
        PackingCluster::bins_ = aBins;
    };

    void addUnfittedItem(int itemKey)
    {
        PackingCluster::bins_.back().Bin::addUnfittedItem(itemKey);
    };

    double getTotalVolumeUtilizationPercentage()
    {
        double actualVolumeUtil = 0;
        for (auto &b : PackingCluster::bins_)
        {
            actualVolumeUtil += b.Bin::getActVolumeUtil();
        };
        return std::max(0.0, actualVolumeUtil / (PackingCluster::requestedBinWidth_ * PackingCluster::requestedBinDepth_ * PackingCluster::requestedBinHeight_ * PackingCluster::bins_.size()) * 100);
    };

    double getTotalWeightUtilizationPercentage()
    {
        double actualWeightUtil = 0;
        for (auto &b : PackingCluster::bins_)
        {
            actualWeightUtil += b.Bin::getActWeightUtil();
        };
        return std::max(0.0, actualWeightUtil / (PackingCluster::requestedBinMaxWeight_ * PackingCluster::bins_.size()) * 100);
    };
    /**
     * @brief Return an integer representing the estimated number of items that will fit a empty bin.
     *
     * This method iterates over the itemKeys vector and makes an estimation of how many of these items will fit into an empty bin.
     * Estimation is fully based on volume.
     *
     * Used to reserve memory and help calibrate the pre-generated tree depth.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    int estimatedNumberOfItemsThatWillFitIntoBin(std::vector<int> &aItemsToBePacked)
    {
        int estimatedNumberOfItems = 0;
        double cumulativeItemVolume = 0.0;
        for (auto itemKey : aItemsToBePacked)
        {
            if ((PackingCluster::masterItemRegister_->ItemRegister::getItem(itemKey).Item::volume_ + cumulativeItemVolume) < PackingCluster::requestedBinMaxVolume_)
            {
                estimatedNumberOfItems += 1;
                cumulativeItemVolume += PackingCluster::masterItemRegister_->ItemRegister::getItem(itemKey).Item::volume_;
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

        PackingCluster::bins_.push_back(Bin(PackingCluster::requestedBinType_,
                                            (PackingCluster::bins_.size() + 1),
                                            PackingCluster::requestedBinWidth_,
                                            PackingCluster::requestedBinDepth_,
                                            PackingCluster::requestedBinHeight_,
                                            PackingCluster::requestedBinMaxWeight_,
                                            PackingCluster::masterGravity_,
                                            PackingCluster::masterItemRegister_,
                                            PackingCluster::estimatedNumberOfItemsThatWillFitIntoBin(aItemsToBePacked)));

        for (auto &itemToPackKey : aItemsToBePacked)
        {
            Item *itemToPack = &PackingCluster::masterItemRegister_->ItemRegister::getItem(itemToPackKey);

            /* Check if item would exceed weight or volume limit. */
            if ((PackingCluster::bins_.back().Bin::getActVolumeUtil() + itemToPack->Item::volume_) > PackingCluster::bins_.back().Bin::volume_ ||
                (PackingCluster::bins_.back().Bin::getActWeightUtil() + itemToPack->Item::weight_) > PackingCluster::bins_.back().Bin::maxWeight_)
            {
                PackingCluster::addUnfittedItem(itemToPack->Item::transientSysId_);
                continue;
            };

            /* Check if item would be the first item in the bin, if so take shortcut. */
            if (PackingCluster::bins_.back().Bin::getFittedItems().empty())
            {
                if (PackingCluster::bins_.back().Bin::placeItemInBin(itemToPackKey))
                {
                    PackingCluster::bins_.back().Bin::updateWithNewFittedItem(itemToPackKey, 0);
                    continue;
                };
            };

            /* Check if item is the same as previous unfitted item. */
            if (PackingCluster::equalsPreviousUnfittedItem(itemToPack))
            {
                PackingCluster::addUnfittedItem(itemToPack->Item::transientSysId_);
                continue;
            }

            PackingCluster::bins_.back().Bin::findItemPosition(itemToPackKey);
        };

        /* Delete the created bin if it contains no items. */
        if (aItemsToBePacked.size() == PackingCluster::bins_.back().Bin::getUnfittedItems().size())
        {
            PackingCluster::deleteLastBin();
            return;
        };

        PackingCluster::startPacking(PackingCluster::bins_.back().Bin::getUnfittedItems());
    };
};

#endif