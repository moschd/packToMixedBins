#ifndef ITEM_REGISTER_H
#define ITEM_REGISTER_H

struct ItemRegister
{
private:
    std::string sortMethod_;
    std::vector<std::shared_ptr<Item>> completeItemVector_;
    std::unordered_map<int, std::shared_ptr<Item>> completeItemMap_;

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
        else if (aSortMethod == constants::itemRegister::parameter::VOLUME)
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::VOLUME;
        }
        else
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::OPTIMIZED;
        };
    }

    /**
     * @brief Create sorted item input vectors split by consolidation key.
     *
     * Create separete vector for each distinct itemConsKey, the items within each vector are sorted according to the sortMethod argument.
     * Returns a vector of vectors, each inner vector contains itemKeys.
     */
    const std::vector<std::vector<int>> getSortedItemConsKeyVectors(std::vector<std::shared_ptr<Item>> &aItemVector) const
    {
        if (ItemRegister::sortMethod_ == constants::itemRegister::parameter::WEIGHT)
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndWeightSorter());
        }
        else
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndVolumeSorter());
        }

        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors = {std::vector<int>{aItemVector[0]->Item::transientSysId_}};

        for (int idx = 1; idx < aItemVector.size(); idx++)
        {
            if (aItemVector[idx]->itemConsolidationKey_ ==
                ItemRegister::getConstItem(FinalSortedItemConsKeyVectors.back().back())->Item::itemConsolidationKey_)
            {
                FinalSortedItemConsKeyVectors.back().push_back(aItemVector[idx]->Item::transientSysId_);
            }
            else
            {
                FinalSortedItemConsKeyVectors.push_back(std::vector<int>{aItemVector[idx]->Item::transientSysId_});
            };
        };

        return FinalSortedItemConsKeyVectors;
    };

public:
    ItemRegister(std::string aSortMethod, unsigned int nrOfItems)
    {
        ItemRegister::setSortMethod(aSortMethod);
        ItemRegister::completeItemVector_.reserve(nrOfItems);
    }

    const std::string getSortMethod() const { return ItemRegister::sortMethod_; };

    /**
     * @brief Add item to the register.
     *
     * @param item
     */
    inline void addItem(std::shared_ptr<Item> item)
    {
        ItemRegister::completeItemVector_.push_back(item);
        ItemRegister::completeItemMap_.insert(std::make_pair(item->Item::transientSysId_, std::move(item)));
    };

    /**
     * @brief Get a Item object.
     *
     * @param key
     * @return Item&
     */
    inline std::shared_ptr<Item> &getItem(const int key)
    {
        return ItemRegister::completeItemMap_.at(key);
    }

    /**
     * @brief Get a const Item object.
     *
     * @param key
     * @return Item&
     */
    const inline std::shared_ptr<Item> &getConstItem(const int key) const
    {
        return ItemRegister::completeItemMap_.at(key);
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

    /**
     * @brief Checks if two items are equal.
     *
     * @param aItemToCompare1
     * @param aItemToCompare2
     * @return true
     * @return false
     */
    const bool itemsAreEqual(const int aItemToCompare1, const int aItemToCompare2) const
    {
        const std::shared_ptr<Item> itemToCompare1 = ItemRegister::getConstItem(aItemToCompare1);
        const std::shared_ptr<Item> itemToCompare2 = ItemRegister::getConstItem(aItemToCompare2);
        return (itemToCompare2->Item::width_ == itemToCompare1->Item::width_ &&
                itemToCompare2->Item::depth_ == itemToCompare1->Item::depth_ &&
                itemToCompare2->Item::height_ == itemToCompare1->Item::height_ &&
                itemToCompare2->Item::allowedRotations_ == itemToCompare1->Item::allowedRotations_);
    };
};

#endif