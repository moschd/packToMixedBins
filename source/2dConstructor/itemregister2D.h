#ifndef ITEM_REGISTER_2D_H
#define ITEM_REGISTER_2D_H

struct ItemRegister2D
{
private:
    std::vector<Item2D> completeItemVector_;
    std::unordered_map<int, Item2D> completeItemMap_;

public:
    ItemRegister2D(Item2D aBaseItem) : completeItemVector_({}), completeItemMap_({})
    {
        ItemRegister2D::completeItemVector_.reserve(500);
        ItemRegister2D::addItem(aBaseItem);
    }

    /// @brief Get an item from the register.
    /// @param key
    /// @return Item&
    inline Item2D &getItem(const int key) { return ItemRegister2D::completeItemMap_.at(key); }

    /// @brief Get the constant base item. This item is not packed, but acts as a reference to spawn items from.
    /// @return const Item&
    inline const Item2D &getBaseItem() const { return ItemRegister2D::completeItemMap_.at(BASE_ITEM_KEY); }

    /// @brief Get a const instance of an item.
    /// @param key
    /// @return const Item&
    const inline Item2D &getConstItem(const int key) const { return ItemRegister2D::completeItemMap_.at(key); }

    /// @brief Get a new, unique, transient item id.
    /// @return const int
    const int getNewItemId() const { return int(ItemRegister2D::completeItemVector_.size()); }

    /// @brief Add an item to the item register.
    /// @param item
    inline void addItem(const Item2D &item)
    {
        ItemRegister2D::completeItemVector_.push_back(item);
        ItemRegister2D::completeItemMap_.insert({item.Item2D::transientSysId_, item});
    };
};

#endif