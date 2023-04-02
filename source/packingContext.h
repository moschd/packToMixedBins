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
     * @brief Set the estimated avg bin utilization.
     *
     * @param aItemsToBePacked
     */
    void setRequestedBinEstimatedAverages(const std::vector<int> aItemsToBePacked)
    {
        PackingContext::requestedBin_->setEstimatedAverages(aItemsToBePacked, *itemRegister_);
    };

    /**
     * @brief Get the requestedBin.
     *
     * @return const RequestedBin*
     */
    const std::shared_ptr<RequestedBin> getRequestedBin() const
    {
        return PackingContext::requestedBin_;
    };

    /**
     * @brief Get the gravity handler.
     *
     * @return const Gravity*
     */
    const std::shared_ptr<Gravity> getGravity() const
    {
        return PackingContext::gravity_;
    };

    /**
     * @brief Get the item register.
     *
     * @return const ItemRegister*
     */
    const std::shared_ptr<ItemRegister> getItemRegister() const
    {
        return PackingContext::itemRegister_;
    };

    /**
     * @brief Add item to the item register.
     *
     * @param item
     */
    void addItemToRegister(std::shared_ptr<Item> item)
    {
        PackingContext::itemRegister_->addItem(item);
    }

    /**
     * @brief Get an Item object.
     *
     * @param key
     * @return Item&
     */
    inline std::shared_ptr<Item> getModifiableItem(const int key)
    {
        return PackingContext::itemRegister_->getItem(key);
    }

    /**
     * @brief Get a const Item object.
     *
     * @param key
     * @return Item&
     */
    const inline std::shared_ptr<Item> getItem(const int key) const
    {
        return PackingContext::itemRegister_->getConstItem(key);
    }

    /**
     * @brief Get sorted item consolidation key vectors.
     *
     * @param item
     */
    const std::vector<std::vector<int>> getSortedItemConsKeyVectors() const
    {
        return PackingContext::itemRegister_->getAllSortedItemConsKeyVectors();
    }

    /**
     * @brief Checks if the item obeys gravity constraints.
     *
     * @param aItemBeingPlaced
     * @param aItemsInBin
     * @return true
     * @return false
     */
    const bool itemObeysGravity(const std::shared_ptr<Item> &aItemBeingPlaced, const std::vector<int> aItemsInBin) const
    {
        if (!PackingContext::gravity_->gravityEnabled(aItemBeingPlaced))
        {
            return true;
        }

        return PackingContext::gravity_->obeysGravity(aItemBeingPlaced, aItemsInBin, PackingContext::getItemRegister());
    }

    /**
     * @brief Get the desired packing direction of the requested bin.
     *
     * @return const std::array<int, 3>
     */
    const std::array<int, 3> getPackingDirection() const
    {
        return PackingContext::requestedBin_->getPackingDirection();
    }
};

#endif