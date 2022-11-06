#ifndef PACKING_CLUSER_H
#define PACKING_CLUSER_H

class PackingCluster
{
private:
    std::vector<Bin> bins_;
    RequestedBin *requestedBin_;
    Gravity *masterGravity_;
    int binIdCounter_;
    bool distributeItems_;

    /**
     * @brief Checks if the item is the same as the most recent unfitted item.
     *
     * Return false if no item is unfitted yet.
     * Return true if items have the same dimensions, and have the same allowed rotations.
     *
     * @param aItemToCheck
     * @return true
     * @return false
     */
    const bool equalsPreviousUnfittedItem(const Item *aItemToCheck) const
    {
        if (PackingCluster::bins_.back().Bin::getUnfittedItems().empty())
        {
            return false;
        };

        const Item *lastUnfittedItem = &PackingCluster::masterItemRegister_->ItemRegister::getConstItem(PackingCluster::bins_.back().Bin::getUnfittedItems().back());
        return (lastUnfittedItem->Item::width_ == aItemToCheck->Item::width_ &&
                lastUnfittedItem->Item::depth_ == aItemToCheck->Item::depth_ &&
                lastUnfittedItem->Item::height_ == aItemToCheck->Item::height_ &&
                lastUnfittedItem->Item::allowedRotations_ == aItemToCheck->Item::allowedRotations_);
    };

    /**
     * @brief Function to be used when placing the first item inside a bin.
     *
     * @param aItemToPack
     * @return const bool
     */
    const bool fitFirstItem(const Item *aItemToPack)
    {
        if (PackingCluster::bins_.back().Bin::placeItemInBin(aItemToPack->transientSysId_))
        {
            PackingCluster::bins_.back().Bin::updateWithFittedItemHelper(aItemToPack->transientSysId_, 0);
            return true;
        }
        return false;
    };

    /**
     * @brief Checks if adding the item would exceed the bins limits.
     *
     * @param aItemToPack
     * @return true
     * @return false
     */
    const bool wouldExceedLimit(const Item *aItemToPack) const
    {
        return PackingCluster::wouldExceedPhysicalLimit(aItemToPack); //|| PackingCluster::exceedsSelfImposedLimit();
    };

    /**
     * @brief Checks if adding the item would exceed the bins physical limits.
     *
     * @param aItemToPack
     * @return true
     * @return false
     */
    const bool wouldExceedPhysicalLimit(const Item *aItemToPack) const
    {
        const Bin lastBin = PackingCluster::getLastCreatedBin();
        return (lastBin.getActVolumeUtil() + aItemToPack->Item::volume_) > requestedBin_->getMaxVolume() ||
               (lastBin.getActWeightUtil() + aItemToPack->Item::weight_) > requestedBin_->getMaxWeight();
    }

    /**
     * @brief Checks if the bin has exceeded packingCluster' self imposed distribute item limits.
     *
     * Check the dimension with the highest utilization threshold.
     * Only relevant if distributeItems = true.
     *
     * @return true
     * @return false
     */
    const bool exceedsSelfImposedLimit() const
    {

        if (!PackingCluster::distributeItems_)
        {
            return false;
        }

        const Bin lastBin = PackingCluster::getLastCreatedBin();
        if (PackingCluster::requestedBin_->getEstAvgVolumeUtil() < PackingCluster::requestedBin_->getEstAvgWeightUtil())
        {
            return PackingCluster::requestedBin_->getEstAvgWeightUtil() < lastBin.getActWeightUtilizationPercentage();
        }
        else
        {
            return PackingCluster::requestedBin_->getEstAvgVolumeUtil() < lastBin.getActVolumeUtilizationPercentage();
        }
    }

public:
    int id_;
    ItemRegister *masterItemRegister_;

    PackingCluster(unsigned int aId,
                   RequestedBin &aRequestedBin,
                   Gravity &aGravity,
                   ItemRegister &aItemRegister,
                   bool aDistributeItems) : id_(aId),
                                            requestedBin_(&aRequestedBin),
                                            masterGravity_(&aGravity),
                                            masterItemRegister_(&aItemRegister),
                                            distributeItems_(aDistributeItems){};

    /**
     * @brief Get bin by id.
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
     * @brief Set the current bin id
     *
     * Used to keep track of the current counter when multiple packing clusters are created.
     *
     * @param aInteger
     */
    void setBinIdCounter(unsigned int aInteger)
    {
        PackingCluster::binIdCounter_ = aInteger;
    };

    /**
     * @brief Get last bin.
     *
     * @return const std::vector<Bin>&
     */
    const Bin &getLastCreatedBin() const
    {
        return PackingCluster::bins_.back();
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
     * @brief Add to last bin' unfitted items.
     *
     * @param aItem
     */
    void addLastBinUnfittedItem(const Item *aItem)
    {
        PackingCluster::bins_.back().Bin::addUnfittedItem(aItem->transientSysId_);
    };

    /**
     * @brief Return an integer representing the estimated number of items that will fit a empty bin.
     *
     * Used to reserve memory and help calibrate the pre-generated tree depth.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    const int estNrOfItemsInBin(const std::vector<int> &aItemsToBePacked) const
    {
        int volumeEstimatedNumberOfItems = 0;
        int weightEstimatedNumberOfItems = 0;
        double volumeCumulativeValue = 0.0;
        double weightCumulativeValue = 0.0;

        for (auto itemKey : aItemsToBePacked)
        {
            const Item *itemToCheck = &PackingCluster::masterItemRegister_->ItemRegister::getConstItem(itemKey);

            if ((itemToCheck->volume_ + volumeCumulativeValue) < PackingCluster::requestedBin_->getMaxVolume())
            {
                volumeEstimatedNumberOfItems += 1;
                volumeCumulativeValue += itemToCheck->volume_;
            };

            if ((itemToCheck->weight_ + weightCumulativeValue) < PackingCluster::requestedBin_->getMaxWeight())
            {
                weightEstimatedNumberOfItems += 1;
                weightCumulativeValue += itemToCheck->weight_;
            };
        };

        return std::min(volumeEstimatedNumberOfItems, weightEstimatedNumberOfItems);
    };

    /**
     * @brief Prepare the cluster for packing bins.
     *
     * @param aItemsToBePacked
     */
    void startPacking(const std::vector<int> aItemsToBePacked)
    {
        PackingCluster::requestedBin_->setEstimatedAverages(aItemsToBePacked, *PackingCluster::masterItemRegister_);
        PackingCluster::startPackingBins(aItemsToBePacked);
    };

    /**
     * @brief Start to add item(s) into a bin.
     *
     * This method iterates over the item vector and tries to place each item into the bin. If a bin is full it
     * creates a new bin and the process starts over, now the input is the previous bin' unfitted items.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    void startPackingBins(const std::vector<int> aItemsToBePacked)
    {
        if (aItemsToBePacked.empty())
        {
            return;
        };

        PackingCluster::bins_.push_back(Bin(PackingCluster::binIdCounter_,
                                            PackingCluster::requestedBin_,
                                            PackingCluster::masterGravity_,
                                            PackingCluster::masterItemRegister_,
                                            PackingCluster::estNrOfItemsInBin(aItemsToBePacked)));

        for (auto &itemToPackKey : aItemsToBePacked)
        {
            const Item *itemToPack = &PackingCluster::masterItemRegister_->ItemRegister::getConstItem(itemToPackKey);

            if (PackingCluster::wouldExceedLimit(itemToPack) || PackingCluster::equalsPreviousUnfittedItem(itemToPack))
            {
                PackingCluster::addLastBinUnfittedItem(itemToPack);
                continue;
            }

            /* Check if item would be the first item in the bin, if so take shortcut. */
            if (PackingCluster::getLastCreatedBin().Bin::getFittedItems().empty())
            {
                if (PackingCluster::fitFirstItem(itemToPack))
                {
                    continue;
                };
            };

            PackingCluster::bins_.back().Bin::findItemPosition(itemToPackKey);
        };

        /* Delete the created bin if it contains no items. */
        if (PackingCluster::getLastCreatedBin().Bin::getFittedItems().empty())
        {
            PackingCluster::deleteLastBin();
            return;
        }

        PackingCluster::binIdCounter_ += 1;
        PackingCluster::startPackingBins(PackingCluster::bins_.back().Bin::getUnfittedItems());
    };
};

#endif