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

    /**
     * @brief Remove itemKeys from the global items to be packed by iterating over the packed items of the packed bin which has been added last.
     *
     */
    void updateItemsToBePacked()
    {

        std::cout << "Items before: " << BinComposer::itemsToBePacked_.size() << "\n";

        BinComposer::itemsToBePacked_.erase(
            std::remove_if(begin(BinComposer::itemsToBePacked_),
                           end(BinComposer::itemsToBePacked_),
                           [&](int &itemToBePackedKey) -> bool
                           {
                               bool isPacked = false;
                               for (const int packedItem : BinComposer::packedBins_.back()->getFittedItems())
                               {
                                   if (packedItem == itemToBePackedKey)
                                   {
                                       isPacked = true;
                                       break;
                                   };
                               };

                               return isPacked;
                           }),
            end(BinComposer::itemsToBePacked_));

        std::cout << "Items after: " << BinComposer::itemsToBePacked_.size() << "\n";
    }

public:
    BinComposer(std::shared_ptr<ItemRegister> aItemRegister) : masterItemRegister_(aItemRegister)
    {
        BinComposer::mixedBinPackerHandler_ = std::make_shared<MixedBinPackerHandler>();
    };

    /**
     * @brief Add a requested bin, these will be used during packing.
     *
     * @param aRequestedBin
     */
    void addRequestedBin(const std::shared_ptr<RequestedBin> aRequestedBin)
    {
        BinComposer::requestedBins_.push_back(aRequestedBin);
        BinComposer::sortRequestedBinsOnVolume();
    };

    void addPackedBin(std::shared_ptr<Bin> aBin)
    {
        std::cout << "Adding packed bin " << aBin->type_ << " to composer.\n";
        BinComposer::packedBins_.push_back(aBin);
        BinComposer::updateItemsToBePacked();
    };

    void addItem(const int aItemKey) { BinComposer::itemsToBePacked_.push_back(aItemKey); };
    const std::vector<int> getItemsToBePacked() { return BinComposer::itemsToBePacked_; };
    const int getNumberOfBins() const { return (int)BinComposer::packedBins_.size(); };
    const std::vector<std::shared_ptr<Bin>> getPackedBins() const { return BinComposer::packedBins_; };

    /**
     * @brief Get the total volume utilization across bins.
     *
     * @return const double
     */
    const double getTotalVolumeUtilPercentage() const
    {
        double runningUtilSum = 0.0;

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {

            runningUtilSum += bin->getRealActualVolumeUtilPercentage();
        };

        return runningUtilSum / BinComposer::packedBins_.size();
    };

    /**
     * @brief Get the total weight utilization across bins.
     *
     * @return const int
     */
    const double getTotalWeightUtilPercentage() const
    {
        double runningUtilSum = 0.0;

        for (const std::shared_ptr<Bin> &bin : BinComposer::packedBins_)
        {

            runningUtilSum += bin->getRealActualWeightUtilPercentage();
        };

        return runningUtilSum / BinComposer::packedBins_.size();
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

        std::vector<std::shared_ptr<Packer>> processedPackers;

        // Execute packing for each requestedbin size.
        for (std::shared_ptr<RequestedBin> requestedBin : BinComposer::requestedBins_)
        {

            std::shared_ptr<ItemRegister> itemRegister = std::make_shared<ItemRegister>(requestedBin->getItemSortMethod(), BinComposer::itemsToBePacked_.size());

            for (const int aItemKeyToBePacked : BinComposer::itemsToBePacked_)
            {
                std::shared_ptr<Item> copiedItem = std::make_shared<Item>(*BinComposer::getMasterItemRegister()->getConstItem(aItemKeyToBePacked));
                itemRegister->addItem(copiedItem);
                copiedItem->reset();
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

        // Evaluate the packing results and find the winning bin.
        for (const std::shared_ptr<Packer> processedPacker : processedPackers)
        {
            std::cout << processedPacker->getContext()->getRequestedBin()->getType() << " "
                      << processedPacker->getContext()->getRequestedBin()->getMaxVolume() << " "
                      << " nr of bins = " << processedPacker->getNumberOfBins() << "\n";

            // Packer found which requires 1 bin, cannot be beaten so break loop.
            if (processedPacker->getNumberOfBins() == 1)
            {
                winningPacker = processedPacker;
                std::cout << "Here, only 1 bin.\n";
                break;
            }

            // If there is no winningPacker yet, set it to the packer which reached this point.
            if (!winningPacker)
            {

                winningPacker = processedPacker;
                std::cout << "Here0\n";
                std::cout << "Winning " << winningPacker->getBins().size() << "\n";
                continue;
            }

            // Current packer requires less bins than the current winningPacker, set new winner.
            if (processedPacker->getBins().size() < winningPacker->getBins().size())
            {
                std::cout << "Here1\n";
                std::cout << "Winning " << winningPacker->getBins().size() << "\n";
                std::cout << "processedPacker " << processedPacker->getBins().size() << "\n";

                winningPacker = processedPacker;
                continue;
            };

            // // Current packer requires the same amount of bins, check which packer has a higher utilized bin.
            // if (processedPacker->getBins().size() == winningPacker->getBins().size())
            // {
            //     if (BinComposer::mixedBinPackerHandler_->getWinningBin(processedPacker)->getRealActualVolumeUtil() >
            //         BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker)->getRealActualVolumeUtil())
            //     {

            //         std::cout << "Here2\n";
            //         std::cout << "Winning " << BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker)->getRealActualVolumeUtil() << "\n";
            //         std::cout << "processedPacker " << BinComposer::mixedBinPackerHandler_->getWinningBin(processedPacker)->getRealActualVolumeUtil() << "\n";

            //         winningPacker = processedPacker;
            //         continue;
            //     };
            // };
        }

        BinComposer::addPackedBin(BinComposer::mixedBinPackerHandler_->getWinningBin(winningPacker));

        if (!BinComposer::itemsToBePacked_.empty())
        {
            BinComposer::compose();
        }
        else
        {
            std::cout << "All items have been packed into bins.\n";
        }
    }
};

#endif