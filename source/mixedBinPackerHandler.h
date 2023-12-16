#ifndef MIXED_BIN_PACKER_HANDLER_H
#define MIXED_BIN_PACKER_HANDLER_H

/**
 * @brief Handles the packed mixed bins.
 *
 */
class MixedBinPackerHandler
{
public:
    /**
     * @brief Return bin with the highest volume util.
     *
     * @return std::shared_ptr<Bin>
     */
    std::shared_ptr<Bin> getWinningBin(const std::shared_ptr<Packer> aPacker)
    {
        std::vector<std::shared_ptr<Bin>> packedBins = aPacker->getBins();

        // for (const std::shared_ptr<Bin> bin : packedBins)
        // {

        //     std::cout << bin->getRealActualVolumeUtilPercentage() << "\n";
        // };
        std::sort(packedBins.begin(), packedBins.end(), [this](std::shared_ptr<Bin> &binLeft, std::shared_ptr<Bin> &binRight)
                  { return binLeft->getRealActualVolumeUtilPercentage() < binRight->getRealActualVolumeUtilPercentage(); });

        // std::cout << "after:\n";
        // for (const std::shared_ptr<Bin> bin : packedBins)
        // {

        //     std::cout << bin->getRealActualVolumeUtilPercentage() << "\n";
        // };

        return packedBins.back();
    }
};

#endif