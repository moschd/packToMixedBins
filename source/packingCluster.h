#ifndef PACKING_CLUSER_H
#define PACKING_CLUSER_H

class PackingCluster
{
private:
    std::vector<std::shared_ptr<Bin>> bins_;
    std::shared_ptr<PackingContext> context_;
    int binIdCounter_;
    std::vector<int> unfittedItems_;
    bool optimizedPackingCompatible_;

    /**
     * @brief Perform optimized layer packing, return a filtered list of the items to be packed once finished.
     *
     * @param aItemsToBePacked
     * @return std::vector<int>
     */
    std::vector<int> &optimizedLayerPacking(std::vector<int> &aItemsToBePacked)
    {
        bool continueLayingLayers = true;

        std::unique_ptr<ItemPositionConstructor> positionConstructor =
            std::make_unique<ItemPositionConstructor>(PackingCluster::context_, aItemsToBePacked);

        while (positionConstructor->hasPrecalculatedBinAvailable() && continueLayingLayers)
        {
            int heightToIncrement = 0;

            const std::vector<int> relevantItems = positionConstructor->getRelevantItems();

            for (int i = 0; i < positionConstructor->getNumberOfBaseItems(); i++)
            {

                if (i >= (int)relevantItems.size())
                {
                    break;
                };

                if (PackingCluster::wouldExceedLimit(relevantItems[i]))
                {
                    continueLayingLayers = false;
                    break;
                };

                std::shared_ptr<Item> myItem = PackingCluster::context_->getModifiableItem(relevantItems[i]);
                const std::shared_ptr<Item> precalculatedItem = positionConstructor->getBaseItemByIndex(i);

                myItem->position_[constants::axis::WIDTH] = precalculatedItem->position_[constants::axis::WIDTH];
                myItem->position_[constants::axis::DEPTH] = precalculatedItem->position_[constants::axis::DEPTH];
                myItem->position_[constants::axis::HEIGHT] = positionConstructor->getHeightAddition();

                // Add and remove the chosen allowed rotation.
                // This way the item gets packed with the precalculated rotation type.
                myItem->allowedRotations_.insert(0, std::to_string(precalculatedItem->rotationType_));
                const bool fits = PackingCluster::placeItem(myItem->transientSysId_, false);
                myItem->allowedRotations_.erase(0, 1);

                if (fits)
                {
                    PackingCluster::getLastCreatedBin()->addFittedItem(myItem->transientSysId_, 99);
                    aItemsToBePacked.erase(std::remove(aItemsToBePacked.begin(), aItemsToBePacked.end(), myItem->transientSysId_), aItemsToBePacked.end());
                    heightToIncrement = myItem->height_;
                }
                else
                {
                    continueLayingLayers = false;
                    myItem->reset();
                }
            };

            positionConstructor->addToHeightAddition(heightToIncrement);
            positionConstructor->reconfigure(aItemsToBePacked);
        };

        return aItemsToBePacked;
    };

    /**
     * @brief Sets the flag indicating if the packing cluster can use optimized packing order.
     *
     * Optimized packing is currenly not available for back to front packing direction.
     *
     */
    void setOptimizedPackingCompatible()
    {
        PackingCluster::optimizedPackingCompatible_ =
            (PackingCluster::context_->getItemRegister()->getSortMethod() == constants::itemRegister::parameter::sortMethod::OPTIMIZED &&
             PackingCluster::context_->getRequestedBin()->getPackingDirection() == constants::bin::parameter::BOTTOM_UP_ARRAY);
    }

    /**
     * @brief Place item on its current position inside the bin.
     *
     * Use aCommit to add or not to add the item to the bin after evaluation.
     *
     * @param aItemKey
     * @param aCommit
     * @return true
     * @return false
     */
    const bool placeItem(const int aItemKey, const bool aCommit = true)
    {
        const bool itemFits = PackingCluster::bins_.back()->Bin::placeItem(aItemKey);

        if (aCommit)
        {
            itemFits
                ? PackingCluster::bins_.back()->Bin::addFittedItem(aItemKey, 99)
                : PackingCluster::getLastCreatedBin()->addUnfittedItem(aItemKey);
        }

        return itemFits;
    }

    /**
     * @brief Checks if adding the item would exceed the bins limits.
     *  Seperate function because this might be extended later by custom non-physical limits.
     *
     * @param aItemKey
     * @return true
     * @return false
     */
    const bool wouldExceedLimit(const int aItemKey) const
    {
        return PackingCluster::wouldExceedPhysicalLimit(aItemKey);
    };

    /**
     * @brief Checks if adding the item would exceed the bins physical limits.
     *
     * @param aItemKey
     * @return true
     * @return false
     */
    const bool wouldExceedPhysicalLimit(const int aItemKey) const
    {
        const std::shared_ptr<Item> aItemToPack = PackingCluster::context_->getItem(aItemKey);
        return (PackingCluster::getLastCreatedBin()->getRealActualVolumeUtil() + aItemToPack->Item::volume_) > context_->getRequestedBin()->getMaxVolume() ||
               (PackingCluster::getLastCreatedBin()->getRealActualWeightUtil() + aItemToPack->Item::weight_) > context_->getRequestedBin()->getMaxWeight();
    }

    /**
     * @brief Return an integer representing the estimated number of items that will fit a empty bin.
     *
     * Used to reserve memory and help calibrate the pre-generated tree depth.
     *
     * @param aItemsToBePacked  - vector containing itemKeys
     */
    const int estimatedNumberOfItemsToFit(const std::vector<int> &aItemsToBePacked) const
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
     * @brief Delete the last bin from the packing cluster.
     *
     */
    void deleteLastBin()
    {
        PackingCluster::bins_.pop_back();
        PackingCluster::binIdCounter_ -= 1;
    };

    /**
     * @brief Append vector of unfittedItems to packingCluster unfitted items.
     *
     * @param aUnfittedItems
     */
    void addUnfittedItems(const std::vector<int> aUnfittedItems)
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
    void startPackingBins(std::vector<int> aItemsToBePacked)
    {

        /* All items have been packed. */
        if (aItemsToBePacked.empty())
        {
            return;
        };

        /* Create a new bin. */
        PackingCluster::binIdCounter_ += 1;
        PackingCluster::bins_.push_back(
            std::make_shared<Bin>(PackingCluster::binIdCounter_,
                                  PackingCluster::context_,
                                  PackingCluster::estimatedNumberOfItemsToFit(aItemsToBePacked)));

        /**
         * Start packing evaluation process.
         *
         * First check if optimized layer packing is to be used.
         *
         * Then, perform iterative item position searching method on the left-over items.
         *
         */
        if (PackingCluster::optimizedPackingCompatible_ && !PackingCluster::context_->getItemRegister()->containsItemWithStackingStyle(aItemsToBePacked))
        {
            aItemsToBePacked = PackingCluster::optimizedLayerPacking(aItemsToBePacked);
        };

        for (const int itemToPackKey : aItemsToBePacked)
        {

            // std::cout << itemToPackKey << " " << context_->getItemRegister()->getConstItem(itemToPackKey)->id_ << " " << context_->getItemRegister()->getConstItem(itemToPackKey)->transientSysId_ << "\n";
            // checks for weight constraint
            if (PackingCluster::wouldExceedLimit(itemToPackKey))
            {
                PackingCluster::getLastCreatedBin()->addUnfittedItem(itemToPackKey);
            }
            // checks for unfitted items which are the same as current item.
            else if (!PackingCluster::getLastCreatedBin()->getUnfittedItems().empty() &&
                     PackingCluster::context_->getItemRegister()->itemsAreEqual(itemToPackKey, PackingCluster::getLastCreatedBin()->Bin::getUnfittedItems().back()))
            {
                PackingCluster::getLastCreatedBin()->addUnfittedItem(itemToPackKey);
            }
            else if (PackingCluster::getLastCreatedBin()->getFittedItems().empty())
            {
                PackingCluster::placeItem(itemToPackKey);
            }
            else
            {
                PackingCluster::getLastCreatedBin()->searchPositionAndPlaceItem(itemToPackKey);
            }
        };

        /* Delete the created bin if it contains no items. */
        if (PackingCluster::getLastCreatedBin()->getFittedItems().empty())
        {
            PackingCluster::addUnfittedItems(PackingCluster::getLastCreatedBin()->Bin::getUnfittedItems());
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
        PackingCluster::setOptimizedPackingCompatible();
    };

    /**
     * @brief Get all packed bins.
     *
     * @return const std::vector<std::shared_ptr<Bin>>&
     */
    const std::vector<std::shared_ptr<Bin>> &getPackedBins() const { return PackingCluster::bins_; };

    /**
     * @brief Set the current bin id
     *
     * Used to keep track of the current counter when multiple packing clusters are created.
     *
     * @param aInteger
     */
    void setBinIdCounter(const int aInteger) { PackingCluster::binIdCounter_ = aInteger; };

    /**
     * @brief Retrieve current maximum bin id.
     *
     * @return const int
     */
    const int getBinIdCounter() const { return PackingCluster::binIdCounter_; }

    /**
     * @brief Get the unfitted items belonging to this cluster.
     *
     * @return const std::vector<int>
     */
    const std::vector<int> &getUnfittedItems() const { return PackingCluster::unfittedItems_; };

    /**
     * @brief Get last bin.
     *
     * @return const std::vector<Bin>&
     */
    const std::shared_ptr<Bin> &getLastCreatedBin() const { return PackingCluster::bins_.back(); };

    /**
     * @brief Prepare the cluster for packing bins and start.
     *
     * @param aItemsToBePacked
     */
    void startPacking(const std::vector<int> &aItemsToBePacked)
    {
        PackingCluster::context_->getRequestedBin()->setEstimatedAverages(aItemsToBePacked, PackingCluster::context_->getItemRegister());
        PackingCluster::startPackingBins(aItemsToBePacked);
    };
};

#endif