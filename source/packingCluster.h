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
    int binIdCounter_;

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
            return 0;
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
            PackingCluster::bins_.back().Bin::updateWithNewFittedItem(aItemToPack->transientSysId_, 0);
            return 1;
        }
        return 0;
    };

    /**
     * @brief Checks if adding the item would exceed the bins limits.
     *
     * @param aItemToPack
     * @return true
     * @return false
     */
    const bool wouldExceedLimit(const Item *aItemToPack)
    {
        const Bin lastBin = PackingCluster::getLastCreatedBin();
        return (lastBin.getActVolumeUtil() + aItemToPack->Item::volume_) > lastBin.volume_ ||
               (lastBin.getActWeightUtil() + aItemToPack->Item::weight_) > lastBin.maxWeight_;
    }

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
     * @brief Get the total volume util of the whole cluster.
     *
     * @return const double
     */
    const double getTotalVolumeUtilizationPercentage() const
    {
        double actualVolumeUtil = 0;
        for (auto &b : PackingCluster::bins_)
        {
            actualVolumeUtil += b.Bin::getActVolumeUtil();
        };
        return std::max(0.0, actualVolumeUtil / (PackingCluster::requestedBinWidth_ * PackingCluster::requestedBinDepth_ * PackingCluster::requestedBinHeight_ * PackingCluster::bins_.size()) * 100);
    };

    /**
     * @brief Get the total weight util of the whole cluster.
     *
     * @return const double
     */
    const double getTotalWeightUtilizationPercentage() const
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
     * Estimation is fully based on volume.
     *
     * Used to reserve memory and help calibrate the pre-generated tree depth.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    const int estimatedNumberOfItemsThatWillFitIntoBin(const std::vector<int> &aItemsToBePacked) const
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
    void startPacking(const std::vector<int> aItemsToBePacked)
    {
        if (aItemsToBePacked.empty())
        {
            return;
        };

        PackingCluster::bins_.push_back(Bin(PackingCluster::requestedBinType_,
                                            (PackingCluster::binIdCounter_),
                                            PackingCluster::requestedBinWidth_,
                                            PackingCluster::requestedBinDepth_,
                                            PackingCluster::requestedBinHeight_,
                                            PackingCluster::requestedBinMaxWeight_,
                                            PackingCluster::masterGravity_,
                                            PackingCluster::masterItemRegister_,
                                            PackingCluster::estimatedNumberOfItemsThatWillFitIntoBin(aItemsToBePacked)));

        for (auto &itemToPackKey : aItemsToBePacked)
        {
            const Item *itemToPack = &PackingCluster::masterItemRegister_->ItemRegister::getItem(itemToPackKey);

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
        PackingCluster::startPacking(PackingCluster::bins_.back().Bin::getUnfittedItems());
    };

#if DISTRIBUTOR_SUPPORT
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
#endif
};

#endif