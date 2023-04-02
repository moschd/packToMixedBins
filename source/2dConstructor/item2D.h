#ifndef ITEM_2D_H
#define ITEM_2D_H

class Item2D : public GeometricShape2D
{
public:
    int transientSysId_;
    std::string id_;
    double weight_;
    int layer_;

    Item2D(int aSystemId,
         std::string aItemId,
         double aWidth,
         double aDepth,
         double aHeight,
         double aWeight,
         int aLayerId) : transientSysId_(aSystemId),
                         id_(aItemId.size() ? aItemId : "NA"),
                         weight_(aWeight),
                         layer_(aLayerId),
                         GeometricShape2D(aWidth, aDepth, aHeight){};

    /**
     * @brief Changes the attributes of the item according to the desired rotation.
     *
     * @param aRotationType
     */
    inline void rotate(int aRotationType)
    {
        Item2D::rotationType_ = aRotationType;
        Item2D::setNewDimensions();
        Item2D::setFurthestPoints();
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
        Item2D::rotate(constants::rotation::type::WDH);
        Item2D::position_ = constants::START_POSITION;
    }
};

#endif
