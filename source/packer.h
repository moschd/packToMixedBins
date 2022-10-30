#ifndef PACKER_H
#define PACKER_H

class Packer
{
private:
    std::vector<Bin> bins_;
    int estimatedTotalRequiredBins_;

public:
    std::string requestedBinType_;
    double requestedBinWidth_;
    double requestedBinDepth_;
    double requestedBinHeight_;
    double requestedBinMaxWeight_;
    double requestedBinMaxVolume_;
    ItemRegister *masterItemRegister_;
    Gravity *masterGravity_;
    std::string distribute_;
    std::vector<PackingCluster> clusters_;

    Packer(std::string aBinType,
           double aBinWidth,
           double aBinDepth,
           double aBinHeight,
           double aBinMaxWeight,
           Gravity &aGravity,
           ItemRegister &aItemRegister,
           std::string aDistribute) : requestedBinType_(aBinType),
                                      requestedBinWidth_(aBinWidth),
                                      requestedBinDepth_(aBinDepth),
                                      requestedBinHeight_(aBinHeight),
                                      requestedBinMaxWeight_(aBinMaxWeight),
                                      masterGravity_(&aGravity),
                                      masterItemRegister_(&aItemRegister),
                                      distribute_(aDistribute)

    {
        Packer::requestedBinMaxVolume_ = (requestedBinWidth_ * requestedBinDepth_ * requestedBinHeight_);

#if DISTRIUBTE_SUPPORT
        Packer::setDistributeMethod();
#endif
    };

    /**
     * @brief Get packing clusters.
     *
     * @return const std::vector<PackingCluster>
     */
    const std::vector<PackingCluster> getClusters() const
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
     * @brief Get bin object based on id_.
     *
     * @return const Bin&
     */
    const Bin &getBinById(const int binToGet) const
    {
        for (auto &cluster : clusters_)
        {
            for (auto &bin : cluster.PackingCluster::getPackedBins())
            {
                if (bin.id_ == binToGet)
                {
                    return bin;
                }
            };
        };
        /* default, should never happen. */
        return Packer::clusters_.back().PackingCluster::getPackedBins().back();
    };

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
                runningUtilSum += bin.getActVolumeUtilizationPercentage();
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
                runningUtilSum += bin.getActWeightUtilizationPercentage();
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
    void startPacking(std::vector<int> aItemsToBePacked)
    {
        if (aItemsToBePacked.empty())
        {
            return;
        };

        PackingCluster newCluster((Packer::clusters_.size() + 1),
                                  Packer::masterItemRegister_->ItemRegister::getItem(aItemsToBePacked.front()).itemConsolidationKey_,
                                  Packer::requestedBinType_,
                                  Packer::requestedBinWidth_,
                                  Packer::requestedBinDepth_,
                                  Packer::requestedBinHeight_,
                                  Packer::requestedBinMaxWeight_,
                                  Packer::requestedBinMaxVolume_,
                                  *Packer::masterGravity_,
                                  *Packer::masterItemRegister_);

        if (Packer::clusters_.empty())
        {
            newCluster.setBinIdCounter(1);
        }
        else
        {
            newCluster.setBinIdCounter(Packer::clusters_.back().getLastCreatedBin().id_ + 1);
        }
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

#if DISTRIBUTOR_SUPPORT

    /**
     * @brief Set the bin utilization distribution method that will be used for this packer once the packing process is finished.
     *
     */
    void setDistributeMethod()
    {
        std::transform(Packer::distribute_.begin(), Packer::distribute_.end(), Packer::distribute_.begin(), ::toupper);

        if (Packer::distribute_ == constants::distributor::type::WEIGHT)
        {
            Packer::distribute_ = constants::distributor::type::WEIGHT;
        }
        else if (Packer::distribute_ == constants::distributor::type::VOLUME)
        {
            Packer::distribute_ = constants::distributor::type::VOLUME;
        }
        else
        {
            Packer::distribute_ = "";
        }
    };

    /**
     * @brief Requests to have its items distributed among the available bins.
     *
     * @return true
     * @return false
     */
    inline const bool requestsDistribution() const
    {
        return !Packer::distribute_.empty();
    };

    /**
     * @brief Set packing cluster.
     *
     * Used when a cluster has been modified by the distributor.
     *
     */
    void setCluster(int aClusterId, PackingCluster *aCluster)
    {
        for (auto &cluster : clusters_)
        {
            if (cluster.id_ == aClusterId)
            {
                cluster = *aCluster;
            };
        };
    };
#endif
};

#endif