#ifndef MIXED_BIN_PACKER_HANDLER_H
#define MIXED_BIN_PACKER_HANDLER_H

/**
 * @brief Handles the packed mixed bins.
 *
 */
class MixedBinPackerHandler
{
private:
    std::vector<std::shared_ptr<Bin>> bins_;
    std::shared_ptr<Packer> packerToBeProcessed_;

public:
    MixedBinPackerHandler();
    /**
     * @brief Get the total number of bins required.
     *
     * @return const int
     */
    const int getNumberOfBins() const { return (int)MixedBinPackerHandler::bins_.size(); };

    /**
     * @brief Get the total volume utilization across bins.
     *
     * @return const double
     */
    const double getTotalVolumeUtilPercentage() const
    {
        double runningUtilSum = 0.0;

        for (const std::shared_ptr<Bin> &bin : MixedBinPackerHandler::bins_)
        {
            runningUtilSum += bin->getRealActualVolumeUtilPercentage();
        };

        return runningUtilSum / MixedBinPackerHandler::getNumberOfBins();
    };

    /**
     * @brief Get the total weight utilization across bins.
     *
     * @return const int
     */
    const double getTotalWeightUtilPercentage() const
    {
        double runningUtilSum = 0.0;

        for (const std::shared_ptr<Bin> &bin : MixedBinPackerHandler::bins_)
        {
            runningUtilSum += bin->getRealActualWeightUtilPercentage();
        };

        return runningUtilSum / MixedBinPackerHandler::getNumberOfBins();
    };


    /**
     * @brief Check sorting method, return first bin.
     * 
     * @return std::shared_ptr<Bin> 
     */
    std::shared_ptr<Bin> getWinningBin()
    {
        std::vector<std::shared_ptr<Bin>> packedbins = MixedBinPackerHandler::packerToBeProcessed_->getBins();

        std::sort(packedbins.begin(), packedbins.end(), [this](std::shared_ptr<Bin> &binLeft, std::shared_ptr<Bin> &binRight)
                  { return binLeft->getRealActualVolumeUtilPercentage() < binRight->getRealActualVolumeUtilPercentage(); });
    }

    /**
     * @brief Function to add a bin.
     *
     * @param aPackedBin
     */
    void addBin(std::shared_ptr<Bin> aPackedBin) { MixedBinPackerHandler::bins_.push_back(aPackedBin); };

    std::vector<std::shared_ptr<Item>> getItemsLeftToPack()
    {

        std::vector<std::shared_ptr<Item>> allItems = {};
        std::shared_ptr<Bin> winningBin = MixedBinPackerHandler::getWinningBin();

        for (const std::shared_ptr<Item> &aItem : MixedBinPackerHandler::packerToBeProcessed_->getContext()->getItemRegister()->getCompleteItemVector())
        {

            bool isPacked = false;

            for (const int packedItemKey : winningBin->getFittedItems())
                if (aItem->transientSysId_ == packedItemKey)
                {
                    isPacked = true;
                    break;
                };

            if (!isPacked)
            {
                allItems.push_back(aItem);
            };
        };

        return allItems;
    };

    /**
     * @brief Selects and adds the correct packed bin to the result.
     *
     */
    std::vector<std::shared_ptr<Item>> appendWinningBin(const std::shared_ptr<Packer> aPacker)
    {

        MixedBinPackerHandler::packerToBeProcessed_ = aPacker;
        MixedBinPackerHandler::bins_.push_back(MixedBinPackerHandler::getWinningBin());
        return MixedBinPackerHandler::getItemsLeftToPack();
    };
};

#endif