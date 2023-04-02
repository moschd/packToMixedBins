#ifndef NEAT_PACKER_PACKING_ACTION_H
#define NEAT_PACKER_PACKING_ACTION_H

class PackingAction
{
private:
    bool hasActionLeft_;
    int itemRotationAxis_;
    int nrOfExecutedItemsInAction_;
    int maxNrOfItemsToCompleteAction_;

    /// @brief Indicate if the packing action is complete or not.
    void setHasActionLeft() { PackingAction::hasActionLeft_ = (PackingAction::nrOfExecutedItemsInAction_ < PackingAction::maxNrOfItemsToCompleteAction_); };

public:
    PackingAction() : hasActionLeft_(false),
                      itemRotationAxis_(constants::axis::DEPTH),
                      nrOfExecutedItemsInAction_(0),
                      maxNrOfItemsToCompleteAction_(0) {}

    /// @brief Set the next packing action.
    /// @param aAxis
    /// @param aNumberOfItems
    void setNextAction(const int aAxis, const int aNumberOfItems)
    {
        PackingAction::setItemRotationAxis(aAxis);
        PackingAction::setMaxNumberOfItemsInAction(aNumberOfItems);
    }

    /// @brief Set the number of items left for this packing action.
    /// @param aNumber
    void setMaxNumberOfItemsInAction(const int aNumber)
    {
        PackingAction::maxNrOfItemsToCompleteAction_ = aNumber;
        PackingAction::setNrOfExcutedItemActions(aNumber - aNumber);
        PackingAction::setHasActionLeft();
    };

    /// @brief Set the already executed number of items.
    /// @param aNumber
    void setNrOfExcutedItemActions(const int aNumber)
    {
        PackingAction::nrOfExecutedItemsInAction_ = aNumber;
        PackingAction::setHasActionLeft();
    };

    /// @brief Sets the axis that indicates how the items need to be rotated.
    /// @param aAxis
    void setItemRotationAxis(const int aAxis) { PackingAction::itemRotationAxis_ = aAxis; };

    /// @brief Checks if the PackingAction has been fully executed.
    /// @return const bool
    const bool getHasActionsLeft() const { return PackingAction::hasActionLeft_; };

    /// @brief Returns the number of items that this packing action has left.
    /// @return const int
    const int getMaxNumberOfItemsInAction() const { return PackingAction::maxNrOfItemsToCompleteAction_; };

    /// @brief Get the already executed number of items.
    /// @return const int
    const int getNrOfExcutedItemActions() const { return PackingAction::nrOfExecutedItemsInAction_; };

    /// @brief Returns the axis that indicates how the items need to be rotated.
    /// @return const int
    const int getItemRotationAxis() const { return PackingAction::itemRotationAxis_; };

    /// @brief Reset the packing action to initial state.
    void reset()
    {
        PackingAction::hasActionLeft_ = false;
        PackingAction::nrOfExecutedItemsInAction_ = 0;
        PackingAction::maxNrOfItemsToCompleteAction_ = 0;
        PackingAction::itemRotationAxis_ = constants::axis::DEPTH;
    }
};

#endif