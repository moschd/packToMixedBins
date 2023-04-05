#ifndef GRAVITY_H
#define GRAVITY_H
/*
    GRAVITY.

    Handles logic related to gravity.

    - Boxes being stacked in the air can happen since width and depth stacking is checked on all items before height stacking.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

class Gravity
{
private:
    double gravityStrength_;

    /**
     * @brief Get gravityStrength applicable for this item.
     *
     * Item gravityStrength has precedence over global gravityStrength.
     *
     * @param aItemBeingPlaced
     * @return const double
     */
    inline const double activeGravityStrength(const std::shared_ptr<Item> &aItemBeingPlaced) const
    {
        if (aItemBeingPlaced->Item::gravityStrength_ > 0.0)
        {
            return aItemBeingPlaced->Item::gravityStrength_;
        }
        else
        {
            return Gravity::gravityStrength_;
        }
    };

    /**
     * @brief Indicates if the coveredSurfaceArea fulfils the gravity requirement for this item.
     *
     * @param aItemBeingPlaced
     * @param aCoveredSurfaceArea
     * @return true
     * @return false
     */
    inline const bool hasSufficientSurfaceSupport(const std::shared_ptr<Item> &aItemBeingPlaced, const double aCoveredSurfaceArea) const
    {
        return aCoveredSurfaceArea >= activeGravityStrength(aItemBeingPlaced);
    };

public:
    bool highLevelGravityEnabled_;

    Gravity(int aGravityStrengthPercentage) : gravityStrength_(aGravityStrengthPercentage)
    {
        Gravity::highLevelGravityEnabled_ = (gravityStrength_ > 0.0);
    };

    /**
     * @brief Returns true if gravity should be checked for this item.
     *
     * @param item
     * @return true
     * @return false
     */
    inline const bool gravityEnabled(const std::shared_ptr<Item> &aItem) const
    {
        return highLevelGravityEnabled_ || aItem->gravityStrength_ > 0.0;
    }

    /**
     * @brief Checks if gravity is obeyed when placing an item into the bin.
     *
     * Checks if the bottom surface area of an item is sufficiently supported by items which are already inside the bin.
     *
     * 1. Calculate surface area which is being covered by this particular itemInSpace.
     *      X smallest furthestPoint - biggest starting point = overlapping X distance
     *      Y smallest furthestPoint - biggest starting point = overlapping Y distance
     *      Multiply to get surface area of overlap.
     * 2. Calculate total surface area of the aItemBeingPlaced.
     *      width * depth
     * 3. Divide 1. by 2. and multiply by 100 to get coverage percentage.
     * 4. Add percentage to local variable to get running total coverage.
     *      aItemBeingPlaced could be supported by multiple items in the bin.
     * 5. Check if suffiently covered.
     *
     * @param aItemBeingPlaced
     * @param aItemsInBin
     * @param aMyItems
     * @return true
     * @return false
     */
    const bool obeysGravity(const std::shared_ptr<Item> &aItemBeingPlaced,
                            const std::vector<int> aItemsInBin,
                            const std::shared_ptr<ItemRegister> aMyItems) const
    {
        bool gravityFit = false;
        double supportedSurfaceAreaPercentage = 0.0;

        if (aItemBeingPlaced->Item::position_[constants::axis::HEIGHT] == constants::START_POSITION[constants::axis::HEIGHT])
        {
            gravityFit = Gravity::hasSufficientSurfaceSupport(aItemBeingPlaced, 100);
        };

        for (auto const &itemInSpaceKey : aItemsInBin)
        {
            const std::shared_ptr<Item> &itemInSpace = aMyItems->ItemRegister::getConstItem(itemInSpaceKey);
            if (aItemBeingPlaced->Item::position_[constants::axis::HEIGHT] != itemInSpace->Item::furthestPointHeight_)
            {
                continue;
            };

            if (gravityFit)
            {
                break;
            };

            if (Geometry::intersectingXY(aItemBeingPlaced, itemInSpace))
            {

                double coveredX = (double)std::max(0,
                                                   (std::min(itemInSpace->Item::furthestPointWidth_, aItemBeingPlaced->Item::furthestPointWidth_) -
                                                    std::max(itemInSpace->Item::position_[constants::axis::WIDTH],
                                                             aItemBeingPlaced->Item::position_[constants::axis::WIDTH]))) /
                                  MULTIPLIER;

                double coveredY = (double)std::max(0,
                                                   (std::min(itemInSpace->Item::furthestPointDepth_, aItemBeingPlaced->Item::furthestPointDepth_) -
                                                    std::max(itemInSpace->Item::position_[constants::axis::DEPTH],
                                                             aItemBeingPlaced->Item::position_[constants::axis::DEPTH]))) /
                                  MULTIPLIER;

                double maxSurface = aItemBeingPlaced->Item::getRealBottomSurfaceArea();

                supportedSurfaceAreaPercentage += coveredX * coveredY / maxSurface * 100;

                gravityFit = Gravity::hasSufficientSurfaceSupport(aItemBeingPlaced, supportedSurfaceAreaPercentage);
            };
        };
        return gravityFit;
    };
};

#endif