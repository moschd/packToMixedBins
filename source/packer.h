#ifndef PACKER_H
#define PACKER_H

class Packer
{
private:
    PackingContext *context_;
    std::vector<PackingCluster> clusters_;

public:
    bool distributeItems_;

    Packer(PackingContext &aContext,
           bool aDistributeItems) : context_(&aContext),
                                    distributeItems_(aDistributeItems){};

    /**
     * @brief Get context.
     *
     * @return const PackingContext*
     */
    const PackingContext *getContext() const
    {
        return Packer::context_;
    }

    /**
     * @brief Modify context.
     *
     * @return const PackingContext*
     */
    PackingContext *getModifiableContext() const
    {
        return Packer::context_;
    }

    /**
     * @brief Get packing clusters.
     *
     * @return const std::vector<PackingCluster>
     */
    const std::vector<PackingCluster> &getClusters() const
    {
        return Packer::clusters_;
    }

    /**
     * @brief Get the total number of bins required.
     *
     * @return const int
     */
    const int getNumberOfBins() const
    {
        int numberOfBins = 0;
        for (auto &cluster : clusters_)
        {
            numberOfBins += cluster.getPackedBins().size();
        };
        return numberOfBins;
    }

    /**
     * @brief Get the total volume utilization across bins.
     *
     * @return const double
     */
    const double getTotalVolumeUtilPercentage() const
    {
        double runningUtilSum = 0.0;
        for (auto &cluster : clusters_)
        {
            for (auto &bin : cluster.getPackedBins())
            {
                runningUtilSum += bin.getActVolumeUtilPercentage();
            };
        };
        return runningUtilSum / Packer::getNumberOfBins();
    };

    /**
     * @brief Get the total weight utilization across bins.
     *
     * @return const double
     */
    const double getTotalWeightUtilPercentage() const
    {
        double runningUtilSum = 0.0;
        for (auto &cluster : clusters_)
        {
            for (auto &bin : cluster.getPackedBins())
            {
                runningUtilSum += bin.getActWeightUtilPercentage();
            };
        };
        return runningUtilSum / Packer::getNumberOfBins();
    };

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

        PackingCluster newCluster(Packer::clusters_.size() + 1, *Packer::context_, Packer::distributeItems_);

        Packer::clusters_.empty() ? newCluster.setBinIdCounter(1)
                                  : newCluster.setBinIdCounter(Packer::clusters_.back().getLastCreatedBin().id_ + 1);

        newCluster.startPacking(aItemsToBePacked);

        Packer::clusters_.push_back(newCluster);
    };

    /**
     * @brief Method called to free memory allocated to the trees for all bins.
     *
     */
    void freeMemory()
    {
        for (auto &cluster : clusters_)
        {
            for (auto &bin : cluster.getPackedBins())
            {
                /* Free memory again, how to do this in a more structured way? */
                bin.Bin::kdTree_->KdTree::deleteAllNodesHelper();
                delete bin.Bin::kdTree_;
            };
        };
    };
};

#endif