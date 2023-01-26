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
    Gravity *gravity_;
    ItemRegister *itemRegister_;
    RequestedBin *requestedBin_;

public:
    PackingContext(Gravity &aGravity,
                   ItemRegister &aItemRegister,
                   RequestedBin &aRequestedBin) : gravity_(&aGravity),
                                                  itemRegister_(&aItemRegister),
                                                  requestedBin_(&aRequestedBin){};

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
    const RequestedBin *getRequestedBin() const
    {
        return PackingContext::requestedBin_;
    };

    /**
     * @brief Get the gravity handler.
     *
     * @return const Gravity*
     */
    const Gravity *getGravity() const
    {
        return PackingContext::gravity_;
    };

    /**
     * @brief Get the item register.
     *
     * @return const ItemRegister*
     */
    const ItemRegister *getItemRegister() const
    {
        return PackingContext::itemRegister_;
    };

    /**
     * @brief Add item to the item register.
     *
     * @param item
     */
    void addItemToRegister(const Item &item)
    {
        PackingContext::itemRegister_->addItem(item);
    }

    /**
     * @brief Get an Item object.
     *
     * @param key
     * @return Item&
     */
    inline Item &getModifiableItem(const int key)
    {
        return PackingContext::itemRegister_->getItem(key);
    }

    /**
     * @brief Get a const Item object.
     *
     * @param key
     * @return Item&
     */
    const inline Item &getItem(const int key) const
    {
        return PackingContext::itemRegister_->getConstItem(key);
    }

    /**
     * @brief Get the number of created items.
     *
     * @return const int
     */
    const inline int getNumberOfCreatedItems() const
    {
        return PackingContext::itemRegister_->numberOfItems();
    };

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
    const bool itemObeysGravity(const Item *aItemBeingPlaced, const std::vector<int> aItemsInBin) const
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