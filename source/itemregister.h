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

        if (aSortMethod == constants::itemRegister::parameter::sortMethod::WEIGHT)
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::sortMethod::WEIGHT;
        }
        else if (aSortMethod == constants::itemRegister::parameter::sortMethod::VOLUME)
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::sortMethod::VOLUME;
        }
        else
        {
            ItemRegister::sortMethod_ = constants::itemRegister::parameter::sortMethod::OPTIMIZED;
        };
    }

    /**
     * @brief Create sorted item input vectors split by consolidation key.
     *
     * Create separete vector for each distinct itemConsKey, the items within each vector are sorted according to the sortMethod argument.
     * Returns a vector of vectors, each inner vector contains itemKeys.
     */
    const std::vector<std::vector<int>> createSortedItemConsKeyVectors(std::vector<std::shared_ptr<Item>> &aItemVector) const
    {

        if (ItemRegister::sortMethod_ == constants::itemRegister::parameter::sortMethod::WEIGHT)
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndWeightSorter());
        }
        else
        {
            std::sort(aItemVector.begin(), aItemVector.end(), consKeyAndVolumeSorter());
        };

        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors = {std::vector<int>{aItemVector[0]->Item::transientSysId_}};

        /**
         * Vector to contain items with this particular stackingStyle, we collect them while iterating and then add them to the front
         * of the vector once all items with that item cons key have been found
         */
        std::vector<int> mustBeBottomNoItemsOnTop = {};

        for (int idx = 1; idx < aItemVector.size(); idx++)
        {

            // If this is true, it means the current item in the loop belongs in the current active vector.
            if (aItemVector[idx]->itemConsolidationKey_ ==
                ItemRegister::getConstItem(FinalSortedItemConsKeyVectors.back().back())->Item::itemConsolidationKey_)
            {

                // If mustBeBottomNoItemsOnTop, we pack the item as very first no matter the volume/weight. So we append this itemKey to the final vector later.
                if (aItemVector[idx]->Item::stackingStyle_ == constants::item::parameter::MUST_BE_BOTTOM_NO_ITEMS_ON_TOP)
                {
                    mustBeBottomNoItemsOnTop.push_back(aItemVector[idx]->Item::transientSysId_);
                }
                else
                {
                    FinalSortedItemConsKeyVectors.back().push_back(aItemVector[idx]->Item::transientSysId_);
                }
            }
            // A new itemConsKey is encountered, we create a new vector.
            else
            {
                // Insert the stackingStyle specific items to the front of the current (finalized) vector.
                FinalSortedItemConsKeyVectors.back().insert(FinalSortedItemConsKeyVectors.back().begin(), mustBeBottomNoItemsOnTop.begin(), mustBeBottomNoItemsOnTop.end());

                // Empty the vector so we can capture new items for the next consKey.
                mustBeBottomNoItemsOnTop.clear();

                // Create next vector.
                FinalSortedItemConsKeyVectors.push_back(std::vector<int>{aItemVector[idx]->Item::transientSysId_});
            };
        };

        // Repeat here one more time because the last item cons key does not reach the 'else' condition written above.
        FinalSortedItemConsKeyVectors.back().insert(FinalSortedItemConsKeyVectors.back().begin(), mustBeBottomNoItemsOnTop.begin(), mustBeBottomNoItemsOnTop.end());
        mustBeBottomNoItemsOnTop.clear();

        return FinalSortedItemConsKeyVectors;
    };

public:
    ItemRegister(std::string aSortMethod, unsigned int nrOfItems)
    {
        ItemRegister::setSortMethod(aSortMethod);
        ItemRegister::completeItemVector_.reserve(nrOfItems);
    }

    const std::string &getSortMethod() const { return ItemRegister::sortMethod_; };

    /// @brief Get a new, unique, transient item id.
    /// @return const int
    const int getNewItemId() const { return int(ItemRegister::completeItemVector_.size()); }

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
    const std::vector<std::vector<int>> getSortedItemConsKeyVectors()
    {
        return ItemRegister::createSortedItemConsKeyVectors(ItemRegister::completeItemVector_);
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
                itemToCompare2->Item::allowedRotations_ == itemToCompare1->Item::allowedRotations_ &&
                itemToCompare2->Item::stackingStyle_ == itemToCompare1->Item::stackingStyle_);
    };

    /**
     * @brief Returns the total volume of the complete packing request.
     *
     * @return const double
     */
    const std::unordered_map<std::string, double> getTotalVolumeMap()
    {
        std::unordered_map<std::string, double> volumeMap;

        for (const std::vector<int> itemVector : ItemRegister::getSortedItemConsKeyVectors())
        {
            const std::string key = ItemRegister::getConstItem(itemVector.front())->itemConsolidationKey_;

            volumeMap[key] = 0.0;
            for (const int aItemKey : itemVector)
            {
                volumeMap[key] += ItemRegister::getConstItem(aItemKey)->getRealVolume();
            }
        };

        return volumeMap;
    };

    /**
     * @brief Checks if any item has a specific stacking style.
     *
     * @return const bool
     */
    const bool containsItemWithStackingStyle(const std::vector<int> aItemKeysToBeChecked) const
    {

        bool containsItem = false;
        for (const int itemKey : aItemKeysToBeChecked)
        {
            if (ItemRegister::getConstItem(itemKey)->stackingStyle_ != constants::item::parameter::ALLOW_ALL)
            {
                containsItem = true;
                break;
            }
        }

        return containsItem;
    }

    /**
     * @brief Checks if any item has the noItemsOnTop stacking style.
     *
     * @return const bool
     */
    const bool containsItemWithNoItemsOnTopStackingStyle(const std::vector<int> aItemKeysToBeChecked) const
    {

        bool containsItem = false;
        for (const int itemKey : aItemKeysToBeChecked)
        {
            if (ItemRegister::getConstItem(itemKey)->stackingStyle_ == constants::item::parameter::NO_ITEMS_ON_TOP)
            {
                containsItem = true;
                break;
            }
        }

        return containsItem;
    }

    /**
     * @brief Checks if any item has the mustBeBottomNoItemsOnTop stacking style.
     *
     * @return const bool
     */
    const bool containsItemWithMustBeBottomNoItemsOnTopStackingStyle(const std::vector<int> aItemKeysToBeChecked) const
    {

        bool containsItem = false;
        for (const int itemKey : aItemKeysToBeChecked)
        {
            if (ItemRegister::getConstItem(itemKey)->stackingStyle_ == constants::item::parameter::MUST_BE_BOTTOM_NO_ITEMS_ON_TOP)
            {
                containsItem = true;
                break;
            }
        }

        return containsItem;
    }
};

#endif