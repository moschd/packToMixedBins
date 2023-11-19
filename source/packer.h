#ifndef PACKER_H
#define PACKER_H

class Packer
{
private:
    std::shared_ptr<PackingContext> context_;
    std::vector<std::shared_ptr<PackingCluster>> clusters_;

public:
    Packer(std::shared_ptr<PackingContext> aContext) : context_(aContext){};

    /**
     * @brief Get context.
     *
     * @return const PackingContext*
     */
    const std::shared_ptr<PackingContext> &getContext() const { return Packer::context_; }

    /**
     * @brief Get packing clusters.
     *
     * @return const std::vector<PackingCluster>
     */
    const std::vector<std::shared_ptr<PackingCluster>> &getClusters() const { return Packer::clusters_; }

    /**
     * @brief Get the total number of bins required.
     *
     * @return const int
     */
    const int getNumberOfBins() const
    {
        int numberOfBins = 0;

        for (const std::shared_ptr<PackingCluster> &cluster : clusters_)
        {
            numberOfBins += (int)cluster->getPackedBins().size();
        };

        return numberOfBins;
    }

        /**
     * @brief Get the total number of bins required.
     *
     * @return const int
     */
    const std::vector<std::shared_ptr<Bin>> getBins() const
    {
        std::vector<std::shared_ptr<Bin>> bins = {};

        for (const std::shared_ptr<PackingCluster> &cluster : clusters_)
        {
            for(std::shared_ptr<Bin> bin : cluster->getPackedBins()){
                bins.push_back(bin);
            };
        };

        return bins;
    }

    // /**
    //  * @brief Get the total volume utilization across bins.
    //  *
    //  * @return const double
    //  */
    // const double getTotalVolumeUtilPercentage() const
    // {
    //     double runningUtilSum = 0.0;

    //     for (const std::shared_ptr<PackingCluster> &cluster : clusters_)
    //     {
    //         for (const std::shared_ptr<Bin> &bin : cluster->getPackedBins())
    //         {
    //             runningUtilSum += bin->getRealActualVolumeUtilPercentage();
    //         };
    //     };

    //     return runningUtilSum / Packer::getNumberOfBins();
    // };

    // /**
    //  * @brief Get the total weight utilization across bins.
    //  *
    //  * @return const int
    //  */
    // const double getTotalWeightUtilPercentage() const
    // {
    //     double runningUtilSum = 0.0;

    //     for (const std::shared_ptr<PackingCluster> &cluster : clusters_)
    //     {
    //         for (const std::shared_ptr<Bin> &bin : cluster->getPackedBins())
    //         {
    //             runningUtilSum += bin->getRealActualWeightUtilPercentage();
    //         };
    //     };
    //     return runningUtilSum / Packer::getNumberOfBins();
    // };

    /**
     * @brief Start packing, create a new cluster per item vector to be packed.
     * This currently means a cluster per item consolidation id but could be interesting to extend this.
     *
     * @param aItemsToBePacked
     */
    void startPackingCluster(std::vector<int> aItemsToBePacked)
    {
        if (aItemsToBePacked.empty())
        {
            return;
        };

        std::shared_ptr<PackingCluster> newCluster = std::make_shared<PackingCluster>((int)Packer::clusters_.size() + 1, Packer::context_);

        if (!Packer::clusters_.empty())
        {
            newCluster->setBinIdCounter(Packer::clusters_.back()->getBinIdCounter());
        };

        newCluster->startPacking(aItemsToBePacked);

        Packer::clusters_.push_back(newCluster);
    };
};

#endif