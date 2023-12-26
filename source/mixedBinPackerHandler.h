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

        std::sort(packedBins.begin(), packedBins.end(), [this](std::shared_ptr<Bin> &binLeft, std::shared_ptr<Bin> &binRight)
                  { return binLeft->getRealActualVolumeUtilPercentage() < binRight->getRealActualVolumeUtilPercentage(); });

        return packedBins.back();
    }

    /**
     * @brief Remove itemKeys from the global items to be packed by iterating over the packed items of the packed bin which has been added last.
     *
     */
    std::vector<int> removeDuplicateIntegers(std::vector<int> aFilteredItems, std::vector<int> aItemsToBeRemoved)
    {
        aFilteredItems.erase(
            std::remove_if(begin(aFilteredItems),
                           end(aFilteredItems),
                           [&](int &itemToBePackedKey) -> bool
                           {
                               bool isDuplicate = false;
                               for (const int packedItem : aItemsToBeRemoved)
                               {
                                   if (packedItem == itemToBePackedKey)
                                   {
                                       isDuplicate = true;
                                       break;
                                   };
                               };
                               return isDuplicate;
                           }),
            end(aFilteredItems));

        return aFilteredItems;
    }
};

#endif