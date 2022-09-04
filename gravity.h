#ifndef GRAVITY_H
#define GRAVITY_H
/*
    GRAVITY.
        Handles logic related to gravity.

    - Boxes being stacked in the air can happen since depth stacking is checked on all items before height stacking.
    - Boxes resting partly on a box.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

class Gravity
{
private:
    double gravityStrengthPercentage_;

public:
    bool gravityEnabled_;

    Gravity(double aGravityStrengthPercentage) : gravityStrengthPercentage_(aGravityStrengthPercentage - 0.1)
    {
        Gravity::gravityEnabled_ = (gravityStrengthPercentage_ > 0.0 ? 1 : 0);
    };

    const bool hasSufficientSurfaceSupport(const Item *itemBeingPlaced,
                                           const std::vector<int> itemsInBin,
                                           const ItemRegister *myItems) const
    {
        bool gravityFit = 0;
        double totalCoveredSurfaceAreaPercentage = 0.0;

        if (itemBeingPlaced->Item::position_[constants::axis::HEIGHT] == constants::START_POSITION[constants::axis::HEIGHT])
        {
            gravityFit = 1;
        };

        for (auto const &itemInSpaceKey : itemsInBin)
        {
            const Item *itemInSpace = &myItems->ItemRegister::getConstItem(itemInSpaceKey);
            if (gravityFit)
            {
                break;
            };
            if (itemBeingPlaced->Item::position_[constants::axis::HEIGHT] != itemInSpace->Item::furthestPointHeight_)
            {
                continue;
            };
            if (Geometry::intersectingXY(itemBeingPlaced, itemInSpace))
            {
                totalCoveredSurfaceAreaPercentage += std::max(0.0, std::min(itemInSpace->Item::furthestPointWidth_, (itemBeingPlaced->Item::position_[constants::axis::WIDTH] + itemBeingPlaced->Item::width_)) - std::max(itemInSpace->Item::position_[constants::axis::WIDTH], itemBeingPlaced->Item::position_[constants::axis::WIDTH])) *
                                                     std::max(0.0, std::min(itemInSpace->Item::furthestPointDepth_, (itemBeingPlaced->Item::position_[constants::axis::DEPTH] + itemBeingPlaced->Item::depth_)) - std::max(itemInSpace->Item::position_[constants::axis::DEPTH], itemBeingPlaced->Item::position_[constants::axis::DEPTH])) / (itemBeingPlaced->Item::width_ * itemBeingPlaced->Item::depth_) * 100;

                gravityFit = totalCoveredSurfaceAreaPercentage >= Gravity::gravityStrengthPercentage_;
            };
        };

        return gravityFit;
    }
};

#endif