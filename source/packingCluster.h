#ifndef PACKING_CLUSER_H
#define PACKING_CLUSER_H

class PackingCluster
{
private:
    std::vector<std::shared_ptr<Bin>> bins_;
    std::shared_ptr<PackingContext> context_;
    int binIdCounter_;
    std::vector<int> unfittedItems_;

    /**
     * @brief Function to be used when placing the first item inside a bin.
     *
     * @param aItemToPackKey
     * @return const bool
     */
    const bool fitFirstItem(const int aItemToPackKey)
    {
        const bool itemFits = PackingCluster::bins_.back()->Bin::placeItemInBin(aItemToPackKey);

        itemFits
            ? PackingCluster::bins_.back()->Bin::updateWithFittedItemHelper(aItemToPackKey, 0)
            : PackingCluster::addLastBinUnfittedItem(aItemToPackKey);

        return itemFits;
    };

    /**
     * @brief Checks if adding the item would exceed the bins limits.
     *  Seperate function because this might be extended later by custom non-physical limits.
     *
     * @param aItemToPackKey
     * @return true
     * @return false
     */
    const bool wouldExceedLimit(const int aItemToPackKey) const
    {
        return PackingCluster::wouldExceedPhysicalLimit(aItemToPackKey);
    };

    /**
     * @brief Checks if adding the item would exceed the bins physical limits.
     *
     * @param aItemToPackKey
     * @return true
     * @return false
     */
    const bool wouldExceedPhysicalLimit(const int aItemToPackKey) const
    {
        const std::shared_ptr<Item> aItemToPack = PackingCluster::context_->getItem(aItemToPackKey);
        return (PackingCluster::getLastCreatedBin()->getActVolumeUtil() + aItemToPack->Item::volume_) > context_->getRequestedBin()->getMaxVolume() ||
               (PackingCluster::getLastCreatedBin()->getActWeightUtil() + aItemToPack->Item::weight_) > context_->getRequestedBin()->getMaxWeight();
    }

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
            const std::shared_ptr<Item> itemToCheck = PackingCluster::context_->getItem(itemKey);

            if ((itemToCheck->volume_ + volumeCumulativeValue) < PackingCluster::context_->getRequestedBin()->getMaxVolume())
            {
                volumeEstimatedNumberOfItems += 1;
                volumeCumulativeValue += itemToCheck->volume_;
            };

            if ((itemToCheck->weight_ + weightCumulativeValue) < PackingCluster::context_->getRequestedBin()->getMaxWeight())
            {
                weightEstimatedNumberOfItems += 1;
                weightCumulativeValue += itemToCheck->weight_;
            };
        };

        return std::min(volumeEstimatedNumberOfItems, weightEstimatedNumberOfItems);
    };

    /**
     * @brief Add to last bin' unfitted items.
     *
     * @param aItemToPackKey
     */
    void addLastBinUnfittedItem(const int aItemToPackKey)
    {
        PackingCluster::bins_.back()->Bin::addUnfittedItem(aItemToPackKey);
    };

    /**
     * @brief Delete the last bin from the packing cluster.
     *
     */
    void deleteLastBin()
    {
        std::shared_ptr<Bin> binToBeDeleted = PackingCluster::bins_.back();
        binToBeDeleted->Bin::kdTree_->KdTree::deleteAllNodesHelper();

        PackingCluster::bins_.pop_back();
        PackingCluster::binIdCounter_ -= 1;
    };

    /**
     * @brief Append vector of unfittedItems to packingCluster unfitted items.
     *
     * @param aUnfittedItems
     */
    void addToUnfittedItems(const std::vector<int> aUnfittedItems)
    {
        PackingCluster::unfittedItems_.insert(PackingCluster::unfittedItems_.end(), aUnfittedItems.begin(), aUnfittedItems.end());
    }

    /**
     * @brief Start to add item(s) into a bin.
     *
     * This method iterates over an item vector and tries to place each item into the bin. If a bin is full it
     * creates a new bin and the process starts over, now the input is the previous bin' unfitted items.
     *
     * If the items to be packed are equal to the bins unfitted items, we've encountered one or more items that can never be packed.
     * These items are stored in the unfitted items of the packingCluster. This prevents unfitted items getting lost when bins are deleted.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    void startPackingBins(const std::vector<int> aItemsToBePacked)
    {

        /* All items have been packed. */
        if (aItemsToBePacked.empty())
        {
            return;
        };

        /* Create a new bin. */
        PackingCluster::binIdCounter_ += 1;
        PackingCluster::bins_.push_back(std::make_shared<Bin>(PackingCluster::binIdCounter_,
                                                              PackingCluster::context_,
                                                              PackingCluster::estNrOfItemsInBin(aItemsToBePacked)));

        /* Start packing evaluation process. */
        bool noItemInBin = true;
        std::unique_ptr<ItemPositionConstructor> positionConstructor = std::make_unique<ItemPositionConstructor>(PackingCluster::context_, aItemsToBePacked);

        if (positionConstructor->hasResult_)
        {
            for (Item2D item : positionConstructor->getItems())
            {
                std::cout << item.id_ << "\n";
            };
        }

        for (auto &itemToPackKey : aItemsToBePacked)
        {
            if (noItemInBin)
            {
                noItemInBin = !PackingCluster::fitFirstItem(itemToPackKey);
            }
            // checks for weight constraint
            else if (PackingCluster::wouldExceedLimit(itemToPackKey))
            {
                PackingCluster::addLastBinUnfittedItem(itemToPackKey);
            }
            // checks for unfitted items which are the same as current item.
            else if (!PackingCluster::bins_.back()->Bin::getUnfittedItems().empty() &&
                     PackingCluster::context_->itemsAreEqual(itemToPackKey, PackingCluster::getLastCreatedBin()->Bin::getUnfittedItems().back()))
            {
                PackingCluster::addLastBinUnfittedItem(itemToPackKey);
            }
            else
            {
                PackingCluster::bins_.back()->Bin::findItemPosition(itemToPackKey);
            }
        };

        /* Delete the created bin if it contains no items. */
        if (noItemInBin)
        {
            PackingCluster::addToUnfittedItems(PackingCluster::getLastCreatedBin()->Bin::getUnfittedItems());
            PackingCluster::deleteLastBin();
            return;
        }

        /* Bin has been packed, recurse. */
        PackingCluster::startPackingBins(PackingCluster::getLastCreatedBin()->Bin::getUnfittedItems());
    };

public:
    int id_;
    PackingCluster(unsigned int aId,
                   std::shared_ptr<PackingContext> aContext) : id_(aId),
                                                               context_(aContext)
    {
        PackingCluster::binIdCounter_ = 0;
    };

    /**
     * @brief Get all packed bins.
     *
     * @return const std::vector<Bin>&
     */
    const std::vector<std::shared_ptr<Bin>> &getPackedBins() const
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
     * @brief Retrieve current maximum bin id.
     *
     * @return const int
     */
    const int getBinIdCounter() const
    {
        return PackingCluster::binIdCounter_;
    }

    /**
     * @brief Get the unfitted items belonging to this cluster.
     *
     * @return const std::vector<int>
     */
    const std::vector<int> &getUnfittedItems() const
    {
        return PackingCluster::unfittedItems_;
    };

    /**
     * @brief Get last bin.
     *
     * @return const std::vector<Bin>&
     */
    const std::shared_ptr<Bin> &getLastCreatedBin() const
    {
        return PackingCluster::bins_.back();
    };

    /**
     * @brief Prepare the cluster for packing bins and start.
     *
     * @param aItemsToBePacked
     */
    void startPacking(const std::vector<int> aItemsToBePacked)
    {
        PackingCluster::context_->setRequestedBinEstimatedAverages(aItemsToBePacked);
        PackingCluster::startPackingBins(aItemsToBePacked);
    };
};

#endif