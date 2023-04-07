#ifndef PACKING_LAYER_H
#define PACKING_LAYER_H

class PackingLayer
{
private:
    int id_;
    std::shared_ptr<PackingContext2D> context_;
    std::array<int, 3> position_;
    std::vector<int> fittedItems_;

    /// @brief Spawn an item on a certain position inside this layer.
    /// @param aXLocation
    /// @param aYLocation
    /// @param aIsRotated
    void spawnItem(const int aXLocation, const int aYLocation, const bool aIsRotated)
    {
        const int newItemId = PackingLayer::context_->getItemRegister()->getNewItemId();
        Item2D newItem(
            newItemId,
            context_->getBaseItem().id_,
            context_->getBaseItem().width_,
            context_->getBaseItem().depth_,
            context_->getBaseItem().height_,
            context_->getBaseItem().weight_,
            PackingLayer::id_,
            context_->getBaseItem().volume_);

        /* Set correct attributes. */
        newItem.position_ = {aXLocation, aYLocation, PackingLayer::position_[constants::axis::HEIGHT]};
        aIsRotated ? newItem.rotate(constants::rotation::type::DWH) : newItem.rotate(constants::rotation::type::WDH);

        /* Add to register and add to this layer. */
        PackingLayer::context_->addItemToRegister(newItem);
        PackingLayer::fittedItems_.push_back(newItemId);
    }

public:
    PackingLayer() : id_(1), context_(), position_(constants::START_POSITION), fittedItems_(){};

    PackingLayer(int aId,
                 std::shared_ptr<PackingContext2D> aContext,
                 std::array<int, 3> aPosition) : id_(aId),
                                                 context_(aContext),
                                                 position_(aPosition),
                                                 fittedItems_()
    {
        PackingLayer::fittedItems_.reserve(25);
    }

    /// @brief Get the fitted items of this layer.
    /// @return const std::vector<int>&
    const std::vector<int> &getFittedItems() const { return PackingLayer::fittedItems_; }

    /// @brief Add item to packing layer. This is a helper method in order not to make the entire function public.
    /// @param aXLocation
    /// @param aYLocation
    /// @param aIsRotated
    void addItem(const int aXLocation,
                 const int aYLocation,
                 const bool aIsRotated) { PackingLayer::spawnItem(aXLocation, aYLocation, aIsRotated); }
};

#endif