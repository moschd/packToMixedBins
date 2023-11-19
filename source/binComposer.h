#ifndef BIN_COMPOSER_H
#define BIN_COMPOSER_H

/**
 * @brief Helps managing the different requested bin objects.
 *
 */
class BinComposer
{
private:
    std::vector<std::shared_ptr<Packer>> packers_;
    std::shared_ptr<MixedBinPackerHandler> mixedBinPackerHandler_;
    bool finalized_;

    /**
     * @brief Sort bins so they get evaluated lowest volume first.
     *
     */
    void sortBinsOnVolume()
    {
        std::sort(packers_.begin(), packers_.end(), [this](std::shared_ptr<Packer> &packerLeft, std::shared_ptr<Packer> &packerRight)
                  { return packerLeft->getContext()->getRequestedBin()->getMaxVolume() < packerRight->getContext()->getRequestedBin()->getMaxVolume(); });
    };

    /**
     * @brief Add a winning bin to the mixed bin handler.
     *
     * @param aPacker
     */
    void addWinningBin(const std::shared_ptr<Packer> aPacker){

    };

public:
    BinComposer(std::vector<std::shared_ptr<Packer>> aPackers) : packers_(aPackers), finalized_(false)
    {
        BinComposer::mixedBinPackerHandler_ = std::make_shared<MixedBinPackerHandler>();
    };

    /**
     * @brief Start constructing winning bins.
     *
     */
    void compose()
    {

        for (std::shared_ptr<Packer> packingProcessor : BinComposer::packers_)
        {

            /* Split items by consolidation key and start packing. */
            for (const std::vector<int> sortedItemConsKeyVector : packingProcessor->getContext()->getItemRegister()->getNewSortedItemKeys())
            {
                packingProcessor->startPackingCluster(sortedItemConsKeyVector);
            };

            std::cout << packingProcessor->getContext()->getRequestedBin()->getType() << " "
                      << packingProcessor->getContext()->getRequestedBin()->getMaxVolume() << " "
                      << " nr of bins = " << packingProcessor->getNumberOfBins() << "\n";

            if (packingProcessor->getNumberOfBins() == 1)
            {
                std::cout << "One bin.\n";
                BinComposer::mixedBinPackerHandler_->appendWinningBin(packingProcessor);
                BinComposer::finalized_ = true;
            };

            if (BinComposer::finalized_)
            {
                std::cout << "Composer is finalized.\n";
                break;
            }
        };

        if(!BinComposer::finalized_){
            
        };
    }
};

#endif