#ifndef ITEM_REGISTER_H
#define ITEM_REGISTER_H

struct ItemRegister
{
private:
    std::string sortMethod_;
    std::vector<Item> completeItemVector_;
    std::unordered_map<int, Item> completeItemMap_;

    /**
     * @brief Set the main sort method for the packer.
     *
     * This method sets the sortMethod_ for the packer, this will determine how items will be sorted before being packed.
     *
     * @param aSortMethod   - string to determine which sort method should be set
     */
    void setSortMethod(std::string aSortMethod)
    {
        std::transform(aSortMethod.begin(), aSortMethod.end(), aSortMethod.begin(), ::toupper);

        if (aSortMethod == constants::itemRegister::parameter::WEIGHT)
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::WEIGHT;
        }
        else
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::VOLUME;
        };
    }

    /**
     * @brief Create sorted item input vectors split by consolidation key.
     *
     * Create separete vector for each distinct itemConsKey, the items within each vector are sorted according to the sortMethod argument.
     * Returns a vector of vectors, each inner vector contains itemKeys.
     */
    const std::vector<std::vector<int>> getSortedItemConsKeyVectors(std::vector<Item> &aItemVector)
    {
        if (ItemRegister::sortMethod_ == constants::itemRegister::parameter::WEIGHT)
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndWeightSorter());
        }
        else
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndVolumeSorter());
        }

        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors = {std::vector<int>{aItemVector[0].Item::transientSysId_}};

        for (int idx = 1; idx < aItemVector.size(); idx++)
        {
            if (aItemVector[idx].itemConsolidationKey_ ==
                ItemRegister::getItem(FinalSortedItemConsKeyVectors.back().back()).Item::itemConsolidationKey_)
            {
                FinalSortedItemConsKeyVectors.back().push_back(aItemVector[idx].Item::transientSysId_);
            }
            else
            {
                FinalSortedItemConsKeyVectors.push_back(std::vector<int>{aItemVector[idx].Item::transientSysId_});
            };
        };
        return FinalSortedItemConsKeyVectors;
    };

public:
    ItemRegister(std::string aSortMethod)
    {
        ItemRegister::setSortMethod(aSortMethod);
    }

    /**
     * @brief Add item to the register.
     *
     * @param item
     */
    inline void addItem(const Item &item)
    {
        ItemRegister::completeItemVector_.push_back(item);
        ItemRegister::completeItemMap_.insert({item.Item::transientSysId_, item});
    };

    /**
     * @brief Get a Item object.
     *
     * @param key
     * @return Item&
     */
    inline Item &getItem(const int key)
    {
        return ItemRegister::completeItemMap_.at(key);
    }

    /**
     * @brief Get a const Item object.
     *
     * @param key
     * @return Item&
     */
    const inline Item &getConstItem(const int key) const
    {
        return ItemRegister::completeItemMap_.at(key);
    }

    /**
     * @brief Get sorted item vectors ready to serve as input for packing process.
     * sorted by sortMethod and itemConsolidationKey.
     *
     * @param aItemKeyVector
     * @return const std::vector<std::vector<int>>
     */
    const std::vector<std::vector<int>> getSortedItemConsKeyVectorsFromTransientIds(std::vector<int> aItemKeyVector)
    {
        std::vector<Item> itemVector;

        for (int itemKey : aItemKeyVector)
        {
            itemVector.push_back(ItemRegister::getItem(itemKey));
        };

        return ItemRegister::getSortedItemConsKeyVectors(itemVector);
    }

    /**
     * @brief Get sorted item vectors ready to serve as input for packing process.
     * sorted by sortMethod and itemConsolidationKey.
     *
     * @return const std::vector<std::vector<int>>
     */
    const std::vector<std::vector<int>> getAllSortedItemConsKeyVectors()
    {
        return ItemRegister::getSortedItemConsKeyVectors(ItemRegister::completeItemVector_);
    };
};

#endif