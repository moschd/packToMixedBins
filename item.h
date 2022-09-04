#ifndef ITEM_H
#define ITEM_H

class Item : public RectangularCuboid
{
private:
    /**
     * @brief Set new dimensions on the item based on its current rotation type.
     *
     * @param aRotationType
     */
    void setNewItemDimensions()
    {
        switch (rotationType_)
        {
        case constants::rotation::type::WDH:
            Item::width_ = Item::original_width_;
            Item::depth_ = Item::Item::original_depth_;
            Item::height_ = Item::Item::original_height_;
            break;
        case constants::rotation::type::DWH:
            Item::width_ = Item::original_depth_;
            Item::depth_ = Item::original_width_;
            Item::height_ = Item::original_height_;
            break;
        case constants::rotation::type::HDW:
            Item::width_ = Item::original_height_;
            Item::depth_ = Item::original_depth_;
            Item::height_ = Item::original_width_;
            break;
        case constants::rotation::type::DHW:
            Item::width_ = Item::original_depth_;
            Item::depth_ = Item::original_height_;
            Item::height_ = Item::original_width_;
            break;
        case constants::rotation::type::HWD:
            Item::width_ = Item::original_height_;
            Item::depth_ = Item::original_width_;
            Item::height_ = Item::original_depth_;
            break;
        case constants::rotation::type::WHD:
            Item::width_ = Item::original_width_;
            Item::depth_ = Item::original_height_;
            Item::height_ = Item::original_depth_;
            break;
        };
    };

    void setRotationTypeDesc()
    {
        switch (Item::rotationType_)
        {
        case constants::rotation::type::WDH:
            Item::rotationTypeDescription_ = "No box rotation";
            break;
        case constants::rotation::type::DWH:
            Item::rotationTypeDescription_ = "Rotate the box around the z-axis by 90°";
            break;
        case constants::rotation::type::HDW:
            Item::rotationTypeDescription_ = "Rotate the box around the x-axis by 90°";
            break;
        case constants::rotation::type::DHW:
            Item::rotationTypeDescription_ = "Rotate the box around the x-axis by 90° and then around the z-axis by 90°";
            break;
        case constants::rotation::type::WHD:
            Item::rotationTypeDescription_ = "Rotate the box around the y-axis by 90°";
            break;
        case constants::rotation::type::HWD:
            Item::rotationTypeDescription_ = "Rotate the box around the z-axis by 90° and then around the x-axis by 90°";
            break;
        };
    };

    /**
     * @brief Determines the furthest point in space of this item.
     *
     */
    inline void setItemFurthestPoints()
    {
        Item::furthestPointWidth_ = Item::position_[constants::axis::WIDTH] + Item::width_;
        Item::furthestPointDepth_ = Item::position_[constants::axis::DEPTH] + Item::depth_;
        Item::furthestPointHeight_ = Item::position_[constants::axis::HEIGHT] + Item::height_;
    };

public:
    int transientSysId_;
    std::string id_;
    double original_width_;
    double original_height_;
    double original_depth_;
    double weight_;
    std::array<double, 3> position_;
    std::string itemConsolidationKey_;
    std::string allowedRotations_;
    int rotationType_;
    std::string rotationTypeDescription_;

    double furthestPointWidth_;
    double furthestPointDepth_;
    double furthestPointHeight_;

    Item(int aSystemId,
         std::string aItemId,
         double aWidth,
         double aDepth,
         double aHeight,
         double aWeight,
         std::string aItemConsKey,
         std::string aAllowedRotations) : transientSysId_(aSystemId),
                                          position_(constants::START_POSITION),
                                          original_width_(aWidth),
                                          original_depth_(aDepth),
                                          original_height_(aHeight),
                                          weight_(aWeight),
                                          itemConsolidationKey_(aItemConsKey),
                                          rotationType_(constants::rotation::type::WDH),
                                          rotationTypeDescription_(""),
                                          RectangularCuboid(aWidth,
                                                            aDepth,
                                                            aHeight)
    {
        id_ = aItemId.size() ? aItemId : "NA";
        allowedRotations_ = aAllowedRotations.size() ? aAllowedRotations : "012345";
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
        Item::setNewItemDimensions();
        Item::setItemFurthestPoints();
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
