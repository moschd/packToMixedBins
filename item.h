#ifndef ITEM_H
#define ITEM_H
#include "constants.h"

class Item
{
public:
    int transientSysId_;
    std::string id_;
    double width_;
    double depth_;
    double height_;
    double original_width_;
    double original_height_;
    double original_depth_;
    double weight_;
    double volume_;
    std::array<double, 3> position_;
    std::string itemConsolidationKey_;
    std::string allowedRotations_;
    int rotationType_;
    std::string rotationTypeDescription_;

    double ipwf_;
    double ipdf_;
    double iphf_;

    Item(int aSystemId, std::string aItemId, double aWidth, double aDepth, double aHeight, double aWeight,
         std::string aItemConsKey, std::string aAllowedRotations)
    {
        transientSysId_ = aSystemId;
        id_ = aItemId.size() ? aItemId : "NA";
        original_width_ = width_ = aWidth;
        original_depth_ = depth_ = aDepth;
        original_height_ = height_ = aHeight;
        volume_ = aWidth * aDepth * aHeight;
        weight_ = aWeight;
        itemConsolidationKey_ = aItemConsKey;
        allowedRotations_ = aAllowedRotations.size() ? aAllowedRotations : "012345";

        rotationType_ = 0;
        rotationTypeDescription_ = "";
        position_ = constants::START_POSITION;
    };

    inline void SetNewItemDimensions(const int aRotationType)
    {
        switch (aRotationType)
        {
        case constants::RT_WDH: // keep {width_, depth, height_};
            width_ = original_width_;
            depth_ = original_depth_;
            height_ = original_height_;
            break;
        case constants::RT_DWH: // {depth, width_, height_};
            width_ = original_depth_;
            depth_ = original_width_;
            height_ = original_height_;
            break;
        case constants::RT_HDW: // {height_, depth, width_};
            width_ = original_height_;
            depth_ = original_depth_;
            height_ = original_width_;
            break;
        case constants::RT_DHW: // {depth, height_, width_};
            width_ = original_depth_;
            depth_ = original_height_;
            height_ = original_width_;
            break;
        case constants::RT_HWD: // {height_, width_, depth};
            width_ = original_height_;
            depth_ = original_width_;
            height_ = original_depth_;
            break;
        case constants::RT_WHD: // {width_, height_, depth};
            width_ = original_width_;
            depth_ = original_height_;
            height_ = original_depth_;
            break;
        default:
            width_ = original_width_;
            depth_ = original_depth_;
            height_ = original_height_;
        };
    };

    inline void SetItemDimensionInfo()
    {
        ipwf_ = position_[0] + width_;
        ipdf_ = position_[1] + depth_;
        iphf_ = position_[2] + height_;
    };

    void SetRotationTypeDesc()
    {
        switch (rotationType_)
        {
        case constants::RT_WDH:
            rotationTypeDescription_ = "No box rotation";
            break;
        case constants::RT_DWH:
            rotationTypeDescription_ = "Rotate the box around the z-axis by 90°";
            break;
        case constants::RT_HDW:
            rotationTypeDescription_ = "Rotate the box around the x-axis by 90°";
            break;
        case constants::RT_DHW:
            rotationTypeDescription_ = "Rotate the box around the x-axis by 90° and then around the z-axis by 90°";
            break;
        case constants::RT_WHD:
            rotationTypeDescription_ = "Rotate the box around the y-axis by 90°";
            break;
        case constants::RT_HWD:
            rotationTypeDescription_ = "Rotate the box around the z-axis by 90° and then around the x-axis by 90°";
            break;
        };
    };
};

#endif