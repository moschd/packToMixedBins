#ifndef PACKING_CONTEXT_2D_H
#define PACKING_CONTEXT_2D_H

#include "itemregister2D.h"
#include "requestedBin2D.h"

/**
 * @brief Represents the context in which the bins will be packed.
 *
 * This is passed onto the different packing entities.
 *
 */
class PackingContext2D
{
private:
    std::shared_ptr<ItemRegister2D> ItemRegister2D_;
    std::shared_ptr<RequestedBin2D> RequestedBin2D_;

public:
    PackingContext2D(
        std::shared_ptr<ItemRegister2D> aItemRegister2D,
        std::shared_ptr<RequestedBin2D> aRequestedBin2D) : ItemRegister2D_(aItemRegister2D),
                                                           RequestedBin2D_(aRequestedBin2D){};

    /// @brief Get the requested bin.
    /// @return const std::shared_ptr<RequestedBin2D>
    const std::shared_ptr<RequestedBin2D> getRequestedBin() const { return PackingContext2D::RequestedBin2D_; };

    /// @brief Get the item register.
    /// @return const std::shared_ptr<ItemRegister2D>
    const std::shared_ptr<ItemRegister2D> getItemRegister() const { return PackingContext2D::ItemRegister2D_; };

    /// @brief Add an item to the ItemRegister2D.
    /// @param item
    void addItemToRegister(const Item2D &item) { PackingContext2D::ItemRegister2D_->addItem(item); }

    /// @brief Return the constant base item.
    /// @return const Item&
    const Item2D &getBaseItem() const { return PackingContext2D::ItemRegister2D_->getBaseItem(); }

    /// @brief Get a const item.
    /// @param key
    /// @return const Item&
    const inline Item2D &getItem(const int key) const { return PackingContext2D::ItemRegister2D_->getConstItem(key); }
};

#endif