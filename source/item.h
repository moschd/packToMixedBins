#ifndef ITEM_H
#define ITEM_H

class Item : public GeometricShape
{
public:
    std::string id_;
    int transientSysId_;
    double weight_;
    std::string itemConsolidationKey_;
    double gravityStrength_;

    Item(int aSystemId,
         std::string aItemId,
         double aWidth,
         double aDepth,
         double aHeight,
         double aDiameter,
         double aWeight,
         std::string aItemConsKey,
         std::string aAllowedRotations,
         double aGravityStrength) : transientSysId_(aSystemId),
                                    weight_(aWeight),
                                    itemConsolidationKey_(aItemConsKey),
                                    gravityStrength_(aGravityStrength),
                                    GeometricShape(aWidth,
                                                   aDepth,
                                                   aHeight,
                                                   aDiameter,
                                                   aAllowedRotations)
    {
        id_ = aItemId.size() ? aItemId : "NA";
    };

    /**
     * @brief Changes the attributes of the item according to the desired rotation.
     *
     * @param aRotationType
     */
    inline void rotate(int aRotationType)
    {
        Item::rotationType_ = aRotationType;
        Item::setRotationTypeDesc();
        Item::setNewDimensions();
        Item::setFurthestPoints();
    }

    /**
     * @brief Resets the attributes of the item.
     *
     * Used when an item has been evaluated and rotated during packing but didnt manage to be packed.
     * The items attributes must be reset to be correctly evualated for packing later on.
     *
     */
    void reset()
    {
        Item::rotate(constants::rotation::type::WDH);
        Item::position_ = constants::START_POSITION;
    }
};

#endif
