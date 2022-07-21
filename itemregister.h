#ifndef ITEM_REGISTER_H
#define ITEM_REGISTER_H

#include <vector>
#include "item.h"
#include "miscfunctions.h"
#include <unordered_map>

struct ItemRegister
{
private:
    std::vector<Item> completeItemVector_;
    std::unordered_map<int, Item> completeItemMap_;
    int mainSortMethod_;

public:
    ItemRegister(int aMainSortMethod)
    {
        setMainSortMethod(aMainSortMethod);
    }

    void addItem(Item &item)
    {
        completeItemVector_.push_back(item);
        completeItemMap_.insert({item.transientSysId_, item});
    };

    std::vector<Item> &getCompleteItemVector()
    {
        return completeItemVector_;
    }

    Item &getItem(int key)
    {
        return completeItemMap_.at(key);
    }

    void updateItemDetails(int key)
    {
        completeItemMap_.at(key).SetRotationTypeDesc();
        completeItemMap_.at(key).SetItemDimensionInfo();
    };

    /**
     * @brief Set the main sort method for the packer.
     *
     * This method sets the mainSortMethod_ for the packer, this will determine how items will be sorted before being packed.
     *
     * @param aMainSortMethod   - integer to determine which sort method should be set
     */
    void setMainSortMethod(int aMainSortMethod)
    {
        if (aMainSortMethod == constants::parameter::WEIGHT)
        {
            mainSortMethod_ = constants::parameter::WEIGHT;
        }
        else
        {
            mainSortMethod_ = constants::parameter::VOLUME;
        };
    }

    /**
     * @brief Create sorted item input vectors split by consolidation key.
     *
     * Create separete vector for each distinct itemConsKey, the items within each vector are sorted according to the mainSortMethod argument.
     * Returns a vector of vectors, each inner vector contains itemKeys.
     */
    std::vector<std::vector<int>> GetSortedItemConsKeyVectors()
    {
        if (mainSortMethod_ == constants::parameter::WEIGHT)
        {
            std::sort(getCompleteItemVector().begin(), getCompleteItemVector().end(), consKeyAndWeightSorter());
        }
        else
        {
            std::sort(getCompleteItemVector().begin(), getCompleteItemVector().end(), consKeyAndVolumeSorter());
        }

        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors = {std::vector<int>{getCompleteItemVector()[0].transientSysId_}};

        for (int idx = 1; idx < getCompleteItemVector().size(); idx++)
        {
            if (getCompleteItemVector()[idx].itemConsolidationKey_ ==
                getItem(FinalSortedItemConsKeyVectors.back().back()).itemConsolidationKey_)
            {
                FinalSortedItemConsKeyVectors.back().push_back(getCompleteItemVector()[idx].transientSysId_);
            }
            else
            {
                FinalSortedItemConsKeyVectors.push_back(std::vector<int>{getCompleteItemVector()[idx].transientSysId_});
            };
        };
        return FinalSortedItemConsKeyVectors;
    };
};

#endif