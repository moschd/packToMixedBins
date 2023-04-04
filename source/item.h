#ifndef ITEM_H
#define ITEM_H

class Item : public GeometricShape
{
public:
    std::string id_;
    int transientSysId_;
    double weight_;
    std::string itemConsolidationKey_;
    int gravityStrength_;

    Item(int aSystemId,
         std::string aItemId,
         int aWidth,
         int aDepth,
         int aHeight,
         double aWeight,
         std::string aItemConsKey,
         std::string aAllowedRotations,
         int aGravityStrength) : transientSysId_(aSystemId),
                                 weight_(aWeight),
                                 itemConsolidationKey_(aItemConsKey),
                                 gravityStrength_(aGravityStrength),
                                 GeometricShape(aWidth,
                                                aDepth,
                                                aHeight,
                                                aAllowedRotations)
    {
        id_ = aItemId.size() ? aItemId : "NA";
    };

    const double getRealWeight() const { return weight_; };

    /**
     * @brief Changes the attributes of the item according to the desired rotation.
     *
     * @param aRotationType
     */
    inline void rotate(int aRotationType)
    {
        Item::rotationType_ = aRotationType;
        Item::setNewDimensions();
        Item::setFurthestPoints();
    }

    /**
     * @brief Resets the attributes of the item.
     *
     * Used when an item has been evaluated and rotated during packing but didnt manage to be packed.
     * The items attributes must be reset to be correctly evualated for packing later on, or for being returned as an unfit item.
     *
     */
    void reset()
    {
        Item::rotate(constants::rotation::type::WDH);
        Item::position_ = constants::START_POSITION;
    }
};

#endif
