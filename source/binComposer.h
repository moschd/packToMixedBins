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

        for (auto const &itemConsKeyVolume : BinComposer::masterItemRegister_->getTotalVolumeMap())
        {
            totalItemVolume += itemConsKeyVolume.second;
        }

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {
            totalAvailableVolume += bin->getRealVolume();
        };

        return totalItemVolume / totalAvailableVolume * 100;
    };

    const double getTotalWeightUtilPercentage() const
    {
        double totalAvailableWeight = 0.0;
        double totalItemWeight = 0.0;

        for (auto const &itemConsKeyWeight : BinComposer::masterItemRegister_->getTotalWeightMap())
        {
            totalItemWeight += itemConsKeyWeight.second;
        }

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {
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
     * @brief Start constructing winning bins.
     *
     */
    void compose()
    {

#if DEBUG
        std::cout << "Starting with nr of items: " << (int)BinComposer::itemsToBePacked_.size() << "\n";
#endif

        std::vector<std::shared_ptr<Packer>> processedPackers;

        // Execute packing for each requestedbin size.
        for (std::shared_ptr<RequestedBin> requestedBin : BinComposer::requestedBins_)
        {

            std::shared_ptr<ItemRegister> itemRegister = std::make_shared<ItemRegister>(requestedBin->getItemSortMethod(), (int)BinComposer::itemsToBePacked_.size());

            for (const int aItemKeyToBePacked : BinComposer::itemsToBePacked_)
            {
                std::shared_ptr<Item> copiedItem = std::make_shared<Item>(*BinComposer::getMasterItemRegister()->getConstItem(aItemKeyToBePacked));
                itemRegister->addItem(copiedItem);
            };

            std::shared_ptr<Gravity> masterGravity = std::make_shared<Gravity>(requestedBin->getBinGravityStrength(), itemRegister);
            std::shared_ptr<Packer> packingProcessor = std::make_shared<Packer>(std::make_shared<PackingContext>(masterGravity, itemRegister, requestedBin));

            for (const std::vector<int> sortedItemConsKeyVector : packingProcessor->getContext()->getItemRegister()->getNewSortedItemKeys())
            {
                packingProcessor->startPackingCluster(sortedItemConsKeyVector);
            };

            processedPackers.push_back(packingProcessor);
        };

        std::shared_ptr<Packer> winningPacker;
        double winningPackerValue = 0;
        // Evaluate the packing results and find the winning bin.
        for (const std::shared_ptr<Packer> processedPacker : processedPackers)
        {
            double processedPackerValue = 0;
#if DEBUG
            std::cout << processedPacker->getContext()->getRequestedBin()->getType() << " "
                      << processedPacker->getContext()->getRequestedBin()->getMaxVolume() << " "
                      << " nr of bins = " << processedPacker->getNumberOfBins() << "\n";
#endif

            // Not all items can be fitted, skip to bigger bin in order to pack these items.
            if (processedPacker->hasUnfittedItems())
            {
#if DEBUG
                std::cout << "Has unfitted items, skipping."
                          << "\n";
#endif
                continue;
            }

            if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::BINS)
            {
#if DEBUG
                std::cout << "Minimization strategy is BINS."
                          << "\n";
#endif
                // Packer found which requires 1 bin, cannot be beaten so break loop.
                if (processedPacker->getNumberOfBins() == 1)
                {
#if DEBUG
                    std::cout << "Requires 1 bin, is winner."
                              << "\n";
#endif
                    winningPacker = processedPacker;
                    break;
                }
            }

            // If there is no winningPacker yet, set it to the packer which reached this point.
            if (!winningPacker && !processedPacker->getBins().empty())
            {
#if DEBUG
                std::cout << "First bin, so setting it to winner.\n";
#endif
                winningPacker = processedPacker;
                continue;
            }

            if (BinComposer::minimizationStrategy_ == constants::binComposer::minimizationStrategy::BINS)
            {
                winningPackerValue = (double)winningPacker->getBins().size();
                processedPackerValue = (double)processedPacker->getBins().size();
            }
            else
            {
                winningPackerValue = winningPacker->getBins().size() * winningPacker->getContext()->getRequestedBin()->getMaxVolume();
                processedPackerValue = processedPacker->getBins().size() * processedPacker->getContext()->getRequestedBin()->getMaxVolume();
            }
#if DEBUG
            std::cout << "Compare values are: winningPacker " << winningPackerValue << " processedPacker " << processedPackerValue << "\n";
#endif

            // Current packer requires less bins than the current winningPacker, set new winner.
            if (winningPacker && processedPackerValue < winningPackerValue)
            {
#if DEBUG
                std::cout << "Setting new winner.\n";
#endif
                winningPacker = processedPacker;
                continue;
            };
        }

        // No items were fitted.
        if (!winningPacker)
        {
            return;
        };

        std::cout << "  Going to pack bin again.\n";
        std::vector<int> fittedItems = BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker)->getFittedItems();
        std::cout << "fitted:\n";
        for (const int a : fittedItems)
        {
            std::cout << a << " ";
        }
        std::cout << "\n";
        // Sort method cannot be OPTIMIZED since OPTIMIZED does not have to take into account the order of items in the sortedItemConsKeyVector.
        // but we rely on this order to make sure the new bin is the same as the old bin.
        winningPacker->getContext()->getItemRegister()->setSortMethodHelper(constants::itemRegister::parameter::sortMethod::VOLUME);
        for (std::vector<int> sortedItemConsKeyVector : winningPacker->getContext()->getItemRegister()->getNewSortedItemKeys())
        {

            std::cout << "before:\n";
            for (const int a : sortedItemConsKeyVector)
            {
                std::cout << a << " ";
            }

            if ((int)sortedItemConsKeyVector.size() == (int)fittedItems.size())
            {
                std::cout << "Nothing new.\n";
                continue;
            };
            std::cout << "\n";

            sortedItemConsKeyVector = BinComposer::mixedBinPackerHandler_->removeDuplicateIntegers(sortedItemConsKeyVector, fittedItems);
            std::cout << "After:\n";
            for (const int a : sortedItemConsKeyVector)
            {
                std::cout << a << " ";
            }
            std::cout << "\n";

            fittedItems.insert(fittedItems.end(), sortedItemConsKeyVector.begin(), sortedItemConsKeyVector.end());
            std::cout << "final:\n";
            for (const int a : fittedItems)
            {
                std::cout << a << " ";
            }
            std::cout << "\n";

            winningPacker->startPackingCluster(fittedItems);
        };

        BinComposer::addPackedBin(BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker));

        if (!BinComposer::itemsToBePacked_.empty())
        {
            BinComposer::compose();
        }
    }
};

#endif