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
    int binIndex_;
    int nrOfBins_;
    std::shared_ptr<Packer> composedPacker_;

    void sortBinsOnVolume()
    {

        std::sort(packers_.begin(), packers_.end(), [this](std::shared_ptr<Packer> &packerLeft, std::shared_ptr<Packer> &packerRight)
                  { return packerLeft->getContext()->getRequestedBin()->getMaxVolume() < packerRight->getContext()->getRequestedBin()->getMaxVolume(); });
    }

public:
    BinComposer(std::vector<std::shared_ptr<Packer>> aPackers) : packers_(aPackers)
    {
        BinComposer::sortBinsOnVolume();
    };

    const std::shared_ptr<Packer> getPacker() { return BinComposer::composedPacker_; };

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
                BinComposer::composedPacker_ = packingProcessor;
                break;
            };
        };
    }
};

#endif