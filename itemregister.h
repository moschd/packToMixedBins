#ifndef ITEM_REGISTER_H
#define ITEM_REGISTER_H

struct ItemRegister
{
private:
    std::vector<Item> completeItemVector_;
    std::unordered_map<int, Item> completeItemMap_;
    int mainSortMethod_;

public:
    ItemRegister(int aMainSortMethod)
    {
        ItemRegister::setMainSortMethod(aMainSortMethod);
    }

    void addItem(Item &item)
    {
        ItemRegister::completeItemVector_.push_back(item);
        ItemRegister::completeItemMap_.insert({item.Item::transientSysId_, item});
    };

    std::vector<Item> &getAllItems()
    {
        return ItemRegister::completeItemVector_;
    }

    Item &getItem(int key)
    {
        return ItemRegister::completeItemMap_.at(key);
    }

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
            ItemRegister::mainSortMethod_ = constants::parameter::WEIGHT;
        }
        else
        {
            ItemRegister::mainSortMethod_ = constants::parameter::VOLUME;
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
        if (ItemRegister::mainSortMethod_ == constants::parameter::WEIGHT)
        {
            std::sort(ItemRegister::getAllItems().begin(), ItemRegister::getAllItems().end(), consKeyAndWeightSorter());
        }
        else
        {
            std::sort(ItemRegister::getAllItems().begin(), ItemRegister::getAllItems().end(), consKeyAndVolumeSorter());
        }

        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors = {std::vector<int>{ItemRegister::getAllItems()[0].Item::transientSysId_}};

        for (int idx = 1; idx < ItemRegister::getAllItems().size(); idx++)
        {
            if (ItemRegister::getAllItems()[idx].itemConsolidationKey_ ==
                ItemRegister::getItem(FinalSortedItemConsKeyVectors.back().back()).Item::itemConsolidationKey_)
            {
                FinalSortedItemConsKeyVectors.back().push_back(ItemRegister::getAllItems()[idx].Item::transientSysId_);
            }
            else
            {
                FinalSortedItemConsKeyVectors.push_back(std::vector<int>{ItemRegister::getAllItems()[idx].Item::transientSysId_});
            };
        };
        return FinalSortedItemConsKeyVectors;
    };
};

#endif