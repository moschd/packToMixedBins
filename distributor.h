#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

class Distributor
{
private:
    double desiredUtil_;
    std::vector<Bin> utilSortedBins_;
    std::string distributionMethod_;
    std::vector<int> itemsToBeDistributed_;

    /**
     * @brief Set the desired utilization percentage based on the average of the PackingCluster.
     *
     * This will be the percentage that each bins will try to achieve.
     *
     */
    void setDesiredUtilizationPercentage()
    {
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            Distributor::desiredUtil_ = Distributor::packedCluster_->PackingCluster::getTotalWeightUtilizationPercentage();
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            Distributor::desiredUtil_ = Distributor::packedCluster_->PackingCluster::getTotalVolumeUtilizationPercentage();
        };
    };

    /**
     * @brief Set the desired distribution method.
     *
     */
    void binUtilizationSort()
    {
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            std::sort(Distributor::utilSortedBins_.begin(), Distributor::utilSortedBins_.end(), binWeightUtilSorter());
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            std::sort(Distributor::utilSortedBins_.begin(), Distributor::utilSortedBins_.end(), binVolumeUtilSorter());
        };
    }

    /**
     * @brief gets the type of utilization which is relevant for this distributor.
     *
     * @param bin
     * @return const double
     */
    inline const double getRelevantUtilizationPercentage(const Bin &bin) const
    {
        double relevantUtilizationValue = 0.0;
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            relevantUtilizationValue = bin.getActWeightUtilizationPercentage();
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            relevantUtilizationValue = bin.getActVolumeUtilizationPercentage();
        };
        return relevantUtilizationValue;
    };

    /**
     * @brief method to check if the bin is under utilized.
     *
     * @param bin
     * @return true
     * @return false
     */
    inline const bool isUnderUtilized(const Bin &bin) const
    {
        return Distributor::getRelevantUtilizationPercentage(bin) < Distributor::desiredUtil_;
    };

    /**
     * @brief method to get the relevant utilization value.
     *
     * @param bin
     * @return true
     * @return false
     */
    inline const double getRelevantUtilizationValue(const Bin &bin) const
    {
        double relevantUtilizationValue = 0.0;
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            relevantUtilizationValue = bin.getActWeightUtil();
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            relevantUtilizationValue = bin.getActVolumeUtil();
        };
        return relevantUtilizationValue;
    };

    /**
     * @brief method to get the relevant utilization value.
     *
     * @param item
     * @return const double
     */
    inline const double getRelevantItemValue(const Item &item) const
    {
        double relevantUtilizationValue = 0.0;
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            relevantUtilizationValue = item.weight_;
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            relevantUtilizationValue = item.volume_;
        };
        return relevantUtilizationValue;
    };

    /**
     * @brief method to get the relevant utilization value.
     *
     * @param item
     * @return const double
     */
    inline const double getRelevantMaximumBinValue(const Bin &bin) const
    {
        double relevantUtilizationValue = 0.0;
        if (Distributor::distributionMethod_ == constants::distributor::type::WEIGHT)
        {
            relevantUtilizationValue = bin.maxWeight_;
        }
        else if (Distributor::distributionMethod_ == constants::distributor::type::VOLUME)
        {
            relevantUtilizationValue = bin.volume_;
        };
        return relevantUtilizationValue;
    };

    /**
     * @brief Start placing the gathered items into the under utilized bins.
     *
     */
    void startPlacingItems()
    {
        const double relevantBinMaxUtilValue = Distributor::getRelevantMaximumBinValue(Distributor::utilSortedBins_[0]);

        for (auto &receivingBin : Distributor::utilSortedBins_)
        {
            receivingBin.resetFreeItemVectors();
        };

        for (const int itemKey : Distributor::itemsToBeDistributed_)
        {
            for (auto &receivingBin : Distributor::utilSortedBins_)
            {
                receivingBin.findItemPosition(itemKey);

                std::vector<int> lowBinCurrentFittedItems = receivingBin.getFittedItems();
                std::vector<int>::iterator lowFittedItemIter = std::find(lowBinCurrentFittedItems.begin(),
                                                                         lowBinCurrentFittedItems.end(),
                                                                         itemKey);

                if (lowFittedItemIter != lowBinCurrentFittedItems.end())
                {
                    break;
                }
                else
                {
                    std::vector<int> lowBinCurrentUnfittedItems = receivingBin.getUnfittedItems();
                    std::vector<int>::iterator lowUnfittedItemIter = std::find(lowBinCurrentUnfittedItems.begin(),
                                                                               lowBinCurrentUnfittedItems.end(), itemKey);
                    lowBinCurrentUnfittedItems.erase(lowUnfittedItemIter);
                    receivingBin.setUnfittedItems(lowBinCurrentUnfittedItems);
                };
            };
        };
    };

    /**
     * @brief Function that collects items that can be redistributed across under utilized bins.
     *
     * Iterate over bins, if a bin is under utilized we continue the search.
     *
     */
    void getAndRemoveItemsToBeDistributed()
    {
        const double relevantBinMaxUtilValue = Distributor::getRelevantMaximumBinValue(Distributor::utilSortedBins_[0]);

        for (auto &givingBin : Distributor::utilSortedBins_)
        {
            if (Distributor::isUnderUtilized(givingBin))
            {
                continue;
            };

            double relevantUtilValue = Distributor::getRelevantUtilizationValue(givingBin);
            std::vector<int> binFittedItems = givingBin.getFittedItems();
            for (int itemKeyIndex = binFittedItems.size(); itemKeyIndex--;)
            {
                const int itemKey = binFittedItems[itemKeyIndex];
                const Item *itemInBin = &Distributor::packedCluster_->masterItemRegister_->getConstItem(itemKey);

                relevantUtilValue -= Distributor::getRelevantItemValue(*itemInBin);

                if (relevantUtilValue / relevantBinMaxUtilValue * 100 < Distributor::desiredUtil_)
                {
                    break;
                };

                Distributor::itemsToBeDistributed_.push_back(itemKey);
                givingBin.Bin::deleteItemFromFittedItems(itemKey);
            };
        }
    }

    /**
     * @brief Distribute items across bins.
     *
     */
    void distribute()
    {
        Distributor::getAndRemoveItemsToBeDistributed();
        Distributor::itemsToBeDistributed_ = Distributor::packedCluster_->masterItemRegister_->getSortedItemConsKeyVectorsFromTransientIds(Distributor::itemsToBeDistributed_)[0];
        Distributor::startPlacingItems();
    }

    void finalize()
    {
        std::sort(Distributor::utilSortedBins_.begin(), Distributor::utilSortedBins_.end(), binCreationSorter());
        Distributor::packedCluster_->setBins(Distributor::utilSortedBins_);
    };

public:
    PackingCluster *packedCluster_;

    Distributor(PackingCluster *aPackedCluster,
                std::string aDistributionMethod) : packedCluster_(aPackedCluster),
                                                   distributionMethod_(aDistributionMethod)

    {
        Distributor::utilSortedBins_ = packedCluster_->getPackedBins();
        Distributor::setDesiredUtilizationPercentage();
        Distributor::binUtilizationSort();
        Distributor::distribute();
        Distributor::finalize();
    };

    void utilizationOverview()
    {
        std::cout << "\ncluster " << Distributor::packedCluster_->id_ << " " << Distributor::packedCluster_->consolidationId_ << ""
                  << "volume=" << Distributor::packedCluster_->PackingCluster::getTotalVolumeUtilizationPercentage() << " "
                  << "weight=" << Distributor::packedCluster_->PackingCluster::getTotalWeightUtilizationPercentage() << " "
                  << "aim=" << Distributor::distributionMethod_ << " " << Distributor::desiredUtil_ << "\n";

        for (auto &bin : Distributor::utilSortedBins_)
        {
            std::cout << "bin     " << bin.id_ << " "
                      << "volume=" << bin.getActVolumeUtilizationPercentage() << " "
                      << "weight=" << bin.getActWeightUtilizationPercentage() << " "
                      << "items=" << bin.getFittedItems().size() << "\n";
        };
        std::cout << "\n";
    };
};

#endif