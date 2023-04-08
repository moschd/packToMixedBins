#ifndef PACKING_CONTEXT_H
#define PACKING_CONTEXT_H

#include "itemregister.h"
#include "requestedBin.h"
#include "gravity.h"

/**
 * @brief Represents the context in which the bins will be packed.
 *
 * This is passed onto the different packing entities.
 *
 */
class PackingContext
{
private:
    std::shared_ptr<Gravity> gravity_;
    std::shared_ptr<ItemRegister> itemRegister_;
    std::shared_ptr<RequestedBin> requestedBin_;

public:
    PackingContext(std::shared_ptr<Gravity> aGravity,
                   std::shared_ptr<ItemRegister> aItemRegister,
                   std::shared_ptr<RequestedBin> aRequestedBin) : gravity_(aGravity),
                                                                  itemRegister_(aItemRegister),
                                                                  requestedBin_(aRequestedBin){};

    /**
     * @brief Get the requestedBin.
     *
     * @return const std::shared_ptr<RequestedBin>
     */
    const std::shared_ptr<RequestedBin>& getRequestedBin() const { return PackingContext::requestedBin_; };

    /**
     * @brief Get the gravity handler.
     *
     * @return const Gravity*
     */
    const std::shared_ptr<Gravity>& getGravity() const { return PackingContext::gravity_; };

    /**
     * @brief Get the item register.
     *
     * @return const ItemRegister*
     */
    const std::shared_ptr<ItemRegister>& getItemRegister() const { return PackingContext::itemRegister_; };

    /**
     * @brief Get an Item that can be modified.
     *
     * @param key
     * @return Item&
     */
    inline std::shared_ptr<Item>& getModifiableItem(const int key) { return PackingContext::itemRegister_->getItem(key); }

    /**
     * @brief Get a const Item.
     *
     * @param key
     * @return Item&
     */
    const inline std::shared_ptr<Item>& getItem(const int key) const { return PackingContext::itemRegister_->getConstItem(key); };
};

#endif