#ifndef BIN_COMPOSER_H
#define BIN_COMPOSER_H

/**
 * @brief Helps managing the different requested bin objects.
 *
 */
class BinComposer
{
private:
    std::shared_ptr<MixedBinPackerHandler> mixedBinPackerHandler_;
    std::vector<std::shared_ptr<RequestedBin>> requestedBins_;
    std::vector<std::shared_ptr<Bin>> packedBins_;
    std::shared_ptr<ItemRegister> masterItemRegister_;
    std::vector<int> itemsToBePacked_;
    int minimizationStrategy_;

    void setMinimizationStrategy(const int aStrategy)
    {
        if (constants::binComposer::minimizationStrategy::BINS == aStrategy)
        {
            BinComposer::minimizationStrategy_ = constants::binComposer::minimizationStrategy::BINS;
        }
        else if (constants::binComposer::minimizationStrategy::STRICT_BINS == aStrategy)
        {
            BinComposer::minimizationStrategy_ = constants::binComposer::minimizationStrategy::STRICT_BINS;
        }
        else
        {
            BinComposer::minimizationStrategy_ = constants::binComposer::minimizationStrategy::VOLUME;
        }
    }

    /**
     * @brief Sort requestedBins so they get evaluated lowest volume first.
     *
     */
    void sortRequestedBinsOnVolume()
    {
        std::sort(requestedBins_.begin(),
                  requestedBins_.end(),
                  [this](std::shared_ptr<RequestedBin> &requestedBinLeft, std::shared_ptr<RequestedBin> &requestedBinRight)
                  { return requestedBinLeft->getMaxVolume() < requestedBinRight->getMaxVolume(); });
    };

    void removeRequestedBin(std::shared_ptr<RequestedBin> aRequestedBin)
    {

        bool binFound = false;
        int binIndex = 0;
        for (int idx = BinComposer::requestedBins_.size(); idx--;)
        {
            if (BinComposer::requestedBins_[idx]->getType() == aRequestedBin->getType())
            {
                binFound = true;
                binIndex = idx;
            };
        }

        if (binFound)
        {
            BinComposer::requestedBins_.erase(BinComposer::requestedBins_.begin() + binIndex);
        };
    }

public:
    BinComposer(std::shared_ptr<ItemRegister> aItemRegister, int aMinimizationStrategy) : masterItemRegister_(aItemRegister)
    {
        BinComposer::mixedBinPackerHandler_ = std::make_shared<MixedBinPackerHandler>();
        BinComposer::setMinimizationStrategy(aMinimizationStrategy);
    };

    void addItem(const int aItemKey) { BinComposer::itemsToBePacked_.push_back(aItemKey); };
    const std::vector<int> &getItemsToBePacked() { return BinComposer::itemsToBePacked_; };
    const int getNumberOfBins() const { return (int)BinComposer::packedBins_.size(); };
    const std::vector<std::shared_ptr<Bin>> getPackedBins() const { return BinComposer::packedBins_; };

    void addRequestedBin(const std::shared_ptr<RequestedBin> aRequestedBin)
    {
        BinComposer::requestedBins_.push_back(aRequestedBin);
        BinComposer::sortRequestedBinsOnVolume();
    };

    void addPackedBin(std::shared_ptr<Bin> aBin)
    {
        aBin->id_ = (int)BinComposer::packedBins_.size() + 1;
        BinComposer::packedBins_.push_back(aBin);

        BinComposer::itemsToBePacked_ = BinComposer::mixedBinPackerHandler_->removeDuplicateIntegers(BinComposer::itemsToBePacked_, aBin->getFittedItems());
    };

    const double getTotalVolumeUtilPercentage() const
    {
        double totalAvailableVolume = 0.0;
        double totalItemVolume = 0.0;

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {
            for (const int item : bin->getFittedItems())
            {
                totalItemVolume += BinComposer::masterItemRegister_->getConstItem(item)->getRealVolume();
            };
            totalAvailableVolume += bin->getRealVolume();
        };

        return totalItemVolume / totalAvailableVolume * 100;
    };

    const double getTotalWeightUtilPercentage() const
    {
        double totalAvailableWeight = 0.0;
        double totalItemWeight = 0.0;

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {
            for (const int item : bin->getFittedItems())
            {
                totalItemWeight += BinComposer::masterItemRegister_->getConstItem(item)->getRealWeight();
            };

            totalAvailableWeight += bin->getRealMaxWeight();
        };

        return totalItemWeight / totalAvailableWeight * 100;
    };

    /**
     * @brief Set the master item register, this contains the total items to be packed.
     *
     * @return std::shared_ptr<ItemRegister>
     */
    std::shared_ptr<ItemRegister> getMasterItemRegister() { return BinComposer::masterItemRegister_; };

    /**
     * @brief Get the items which will never fit in any bin.
     *
     * @return const std::vector<int>
     */
    const std::vector<int> getItemsWhichWillNeverFit()
    {
        std::vector<int> itemsWhichWillNeverFit = {};

        for (const int aItemKeyToBePacked : BinComposer::itemsToBePacked_)
        {
            bool canFit = false;

            const std::shared_ptr<Item> itemToFit = BinComposer::getMasterItemRegister()->getConstItem(aItemKeyToBePacked);

            for (const std::shared_ptr<RequestedBin> requestedBin : BinComposer::requestedBins_)
            {

                if (canFit)
                {
                    break;
                }

                if (itemToFit->weight_ > requestedBin->getMaxWeight())
                {
                    continue;
                };

                for (int stringCharCounter = 0; stringCharCounter < itemToFit->Item::allowedRotations_.std::string::size(); stringCharCounter++)
                {
                    itemToFit->Item::rotate(itemToFit->Item::allowedRotations_[stringCharCounter] - '0');

                    /* Check if item is not exceeding the bin dimensions, if so try a different rotation. */
                    if (requestedBin->getWidth() < itemToFit->Item::furthestPointWidth_ ||
                        requestedBin->getDepth() < itemToFit->Item::furthestPointDepth_ ||
                        requestedBin->getHeight() < itemToFit->Item::furthestPointHeight_)
                    {
                        continue;
                    }

                    canFit = true;
                    break;
                };
            }

            itemToFit->reset();

            if (!canFit)
            {
                itemsWhichWillNeverFit.push_back(aItemKeyToBePacked);
            }
        };

        return itemsWhichWillNeverFit;
    };

    void startPacking()
    {
        // Remove items which will never fit and add them back after packing is done..
        std::vector<int> itemsWhichWillNeverFit = BinComposer::getItemsWhichWillNeverFit();
        BinComposer::itemsToBePacked_ = BinComposer::mixedBinPackerHandler_->removeDuplicateIntegers(BinComposer::itemsToBePacked_, itemsWhichWillNeverFit);
        BinComposer::compose();
        BinComposer::itemsToBePacked_.insert(BinComposer::itemsToBePacked_.end(), itemsWhichWillNeverFit.begin(), itemsWhichWillNeverFit.end());
    }

    /**
     * @brief Start constructing winning bins.
     *
     */
    void compose()
    {

        if (BinComposer::itemsToBePacked_.empty())
        {
            return;
        }

        std::vector<std::shared_ptr<Packer>> processedPackers;

        // Execute packing for each requestedbin size.
        for (std::shared_ptr<RequestedBin> requestedBin : BinComposer::requestedBins_)
        {

            std::vector<int> nonCompatibleItems = {};
            std::shared_ptr<ItemRegister> itemRegister = std::make_shared<ItemRegister>(requestedBin->getItemSortMethod(), (int)BinComposer::itemsToBePacked_.size());

            for (const int aItemKeyToBePacked : BinComposer::itemsToBePacked_)
            {

                std::shared_ptr<Item> copiedItem = std::make_shared<Item>(*BinComposer::getMasterItemRegister()->getConstItem(aItemKeyToBePacked));

                bool isBinCompatible = copiedItem->compatibleBins_.empty();
                for (const std::string compatibleBin : BinComposer::getMasterItemRegister()->getConstItem(aItemKeyToBePacked)->compatibleBins_)
                {
                    if (isBinCompatible)
                    {
                        break;
                    };
                    isBinCompatible = (compatibleBin == requestedBin->getType());
                }

                if (!isBinCompatible)
                {
                    nonCompatibleItems.push_back(aItemKeyToBePacked);
                }

                itemRegister->addItem(copiedItem);
            };

            if (itemRegister->getCompleteItemVector().empty())
            {
                continue;
            };

            std::shared_ptr<Gravity> masterGravity = std::make_shared<Gravity>(requestedBin->getBinGravityStrength(), itemRegister);
            std::shared_ptr<Packer> packingProcessor = std::make_shared<Packer>(std::make_shared<PackingContext>(masterGravity, itemRegister, requestedBin));

            for (std::vector<int> sortedItemConsKeyVector : packingProcessor->getContext()->getItemRegister()->getNewSortedItemKeys())
            {
                sortedItemConsKeyVector = BinComposer::mixedBinPackerHandler_->removeDuplicateIntegers(sortedItemConsKeyVector, nonCompatibleItems);
                packingProcessor->startPackingCluster(sortedItemConsKeyVector);
            };

            packingProcessor->getClusters().front()->addUnfittedItemsHelper(nonCompatibleItems);
            processedPackers.push_back(packingProcessor);
        };

        std::shared_ptr<Packer> winningPacker;
        double winningPackerValue = 0;
        // Evaluate the packing results and find the winning bin.
        for (const std::shared_ptr<Packer> processedPacker : processedPackers)
        {
            double processedPackerValue = 0;

            // Not all items can be fitted, skip to bigger bin in order to pack these items. This works because the items which can never fit have been removed previously.
            if (processedPacker->hasUnfittedItems())
            {
                continue;
            }

            if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::BINS)
            {
                // Packer found which requires 1 bin, cannot be beaten so break loop.
                if (processedPacker->getNumberOfBins() == 1)
                {
                    winningPacker = processedPacker;
                    break;
                }
            }

            // If there is no winningPacker yet, set it to the packer which reached this point.
            if (!winningPacker && !processedPacker->getBins().empty())
            {
                winningPacker = processedPacker;
                continue;
            }

            if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::BINS)
            {
                winningPackerValue = (double)winningPacker->getBins().size();
                processedPackerValue = (double)processedPacker->getBins().size();
            }
            else if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::STRICT_BINS)
            {
                winningPackerValue = BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker)->getRealActualVolumeUtilPercentage();
                processedPackerValue = BinComposer::mixedBinPackerHandler_->getWinningBin(processedPacker)->getRealActualVolumeUtilPercentage();
            }
            else
            {
                winningPackerValue = winningPacker->getBins().size() * winningPacker->getContext()->getRequestedBin()->getMaxVolume();
                processedPackerValue = processedPacker->getBins().size() * processedPacker->getContext()->getRequestedBin()->getMaxVolume();
            }

            // Current packer requires less bins than the current winningPacker, set new winner.
            if (winningPacker && processedPackerValue < winningPackerValue)
            {
                winningPacker = processedPacker;
                continue;
            };
        }

        // No items were fitted.
        if (!winningPacker)
        {
            return;
        };

        std::vector<int> fittedItems = BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker)->getFittedItems();
        const std::string consolidationKeyOfFittedItems = BinComposer::masterItemRegister_->getConstItem(fittedItems.front())->itemConsolidationKey_;

        // Sort method cannot be OPTIMIZED since OPTIMIZED does not have to take into account the order of items in the sortedItemConsKeyVector.
        // but we rely on this order to make sure the new bin is the same as the old bin.
        winningPacker->getContext()->getItemRegister()->setSortMethodHelper(constants::itemRegister::parameter::sortMethod::VOLUME);
        std::vector<int> sortedItemConsKeyVector = winningPacker->getContext()->getItemRegister()->getNewSortedItemKeysForKey(consolidationKeyOfFittedItems);

        // If these are equal, then nothing new can be packed.
        if ((int)sortedItemConsKeyVector.size() > (int)fittedItems.size())
        {
            sortedItemConsKeyVector = BinComposer::mixedBinPackerHandler_->removeDuplicateIntegers(sortedItemConsKeyVector, fittedItems);
            fittedItems.insert(fittedItems.end(), sortedItemConsKeyVector.begin(), sortedItemConsKeyVector.end());
            winningPacker->startPackingCluster(fittedItems);
        };

        BinComposer::addPackedBin(BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker));

        if (!BinComposer::itemsToBePacked_.empty())
        {
            if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::STRICT_BINS)
            {
                BinComposer::removeRequestedBin(winningPacker->getContext()->getRequestedBin());
            };

            BinComposer::compose();
        }
    }
};

#endif