#ifndef GRAVITY_H
#define GRAVITY_H

#define OBEYS_GRAVITY true
#define DEFAULT_VERTICE_SUPPORT_THRESHOLD 65.0
#define DEFAULT_VERTICE_DISTANCE_THRESHOLD 50.0

/*
    GRAVITY.

    Handles logic related to gravity.

    - Boxes being stacked in the air can happen since width and depth stacking is checked on all items before height stacking.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

struct SupportPoint
{
    int supportingAxis_;
    int parentItem_;
    std::array<int, 2> earliestTouchingPoint_;
    std::array<int, 2> latestTouchingPoint_;

    /// @brief helper function.
    void printMe() const { std::cout << "Parent item: " << parentItem_
                                     << " supporting: " << supportingAxis_
                                     << " earliest:" << earliestTouchingPoint_[constants::axis::WIDTH] << " " << earliestTouchingPoint_[constants::axis::DEPTH]
                                     << " latest:" << latestTouchingPoint_[constants::axis::WIDTH] << " " << latestTouchingPoint_[constants::axis::DEPTH]
                                     << "\n"; }

    /// @brief Get the opposite axis from where the support was found.
    // Input of 1 returns 0, input of 0 returns 1.
    // Only supports axis of 0 and 1.
    /// @return const int
    const int getOppositeAxis() const { return 1 - supportingAxis_; };

    /// @brief Get the distance between the two supporting points.
    /// @param aSecondPoint
    /// @return const int
    const int supportDistance(const SupportPoint aSecondPoint) const
    {
        const int axis = getOppositeAxis();

        return (std::max(latestTouchingPoint_[axis], aSecondPoint.latestTouchingPoint_[axis]) -
                std::min(earliestTouchingPoint_[axis], aSecondPoint.earliestTouchingPoint_[axis]));
    }
};

class Gravity
{
private:
    double gravityStrength_;
    std::shared_ptr<ItemRegister> itemRegister_;

    /**
     * @brief Checks if the item is placed on the ground, if so, gravity will always be obeyed.
     *
     * @param aItemBeingPlaced
     * @return true
     * @return false
     */
    const bool itemIsPlacedOnGround(const std::shared_ptr<Item> &aItemBeingPlaced) const
    {
        return aItemBeingPlaced->Item::position_[constants::axis::HEIGHT] == constants::START_POSITION[constants::axis::HEIGHT];
    }

    /**
     * @brief Returns a vector of itemkeys indicating which items are crossing the xy plane of the provided box.
     *
     * These items are then used to check for different kind of gravity support types.
     *
     * @param aItemBeingPlaced
     * @param aItemsInBin
     * @param aMyItems
     * @return const std::vector<int>
     */
    const std::vector<int> collectSupportingItems(const std::shared_ptr<Item> &aItemBeingPlaced,
                                                  const std::vector<int> &aItemsInBin) const
    {
        std::vector<int> supportingItems;

        for (auto const &itemInSpaceKey : aItemsInBin)
        {
            const std::shared_ptr<Item> &itemInSpace = Gravity::itemRegister_->ItemRegister::getConstItem(itemInSpaceKey);
            if (aItemBeingPlaced->Item::position_[constants::axis::HEIGHT] == itemInSpace->Item::furthestPointHeight_ &&
                Geometry::intersectingXY(aItemBeingPlaced, itemInSpace))
            {
                supportingItems.push_back(itemInSpace->transientSysId_);
            };
        };

        return supportingItems;
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
     * @brief Checks if gravity is obeyed when placing an item into the bin by validating the covered surface area.
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
     * @param aSupportingItems
     * @param aMyItems
     * @return true
     * @return false
     */
    const bool hasSurfaceSupport(const std::shared_ptr<Item> &aItemBeingPlaced,
                                 const std::vector<int> &aSupportingItems) const
    {
        bool gravityFit = !OBEYS_GRAVITY;
        double supportedSurfaceAreaPercentage = 0.0;

        for (const int &itemInSpaceKey : aSupportingItems)
        {
            const std::shared_ptr<Item> &itemInSpace = Gravity::itemRegister_->ItemRegister::getConstItem(itemInSpaceKey);

            if (gravityFit)
            {
                break;
            };

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

            supportedSurfaceAreaPercentage += coveredX * coveredY / aItemBeingPlaced->Item::getRealBottomSurfaceArea() * 100;

            gravityFit = Gravity::hasSufficientSurfaceSupport(aItemBeingPlaced, supportedSurfaceAreaPercentage);
        };

        return gravityFit;
    };

    const bool hasVerticeSupport(const std::shared_ptr<Item> &aItemBeingPlaced,
                                 const std::vector<int> &aSupportingItems) const
    {
        bool gravityFit = !OBEYS_GRAVITY;

        const int xMin = aItemBeingPlaced->position_[constants::axis::WIDTH];
        const int xMax = aItemBeingPlaced->furthestPointWidth_;
        const int yMin = aItemBeingPlaced->position_[constants::axis::DEPTH];
        const int yMax = aItemBeingPlaced->furthestPointDepth_;

        std::vector<SupportPoint> supportPoints;

        for (const int &itemInSpaceKey : aSupportingItems)
        {

            const std::shared_ptr<Item> &itemInSpace = Gravity::itemRegister_->ItemRegister::getConstItem(itemInSpaceKey);

            int xBottomReach = std::max(xMin, itemInSpace->position_[constants::axis::WIDTH]);
            int xTopReach = std::min(xMax, itemInSpace->furthestPointWidth_);
            const double xCoverage = (double)(xTopReach - xBottomReach) / (xMax - xMin) * 100;
            const bool xAxisCovered = xCoverage >= DEFAULT_VERTICE_SUPPORT_THRESHOLD;

            int yBottomReach = std::max(yMin, itemInSpace->position_[constants::axis::DEPTH]);
            int yTopReach = std::min(yMax, itemInSpace->furthestPointDepth_);
            const double yCoverage = (double)(yTopReach - yBottomReach) / (yMax - yMin) * 100;
            const bool yAxisCovered = yCoverage >= DEFAULT_VERTICE_SUPPORT_THRESHOLD;

            if (xAxisCovered || yAxisCovered)
            {
                SupportPoint supportPoint = {};
                supportPoint.supportingAxis_ = (xAxisCovered ? constants::axis::WIDTH : constants::axis::DEPTH);
                supportPoint.earliestTouchingPoint_ = {xBottomReach, yBottomReach};
                supportPoint.latestTouchingPoint_ = {xTopReach, yTopReach};
                supportPoint.parentItem_ = itemInSpace->transientSysId_;
                supportPoints.push_back(supportPoint);
            };
        };

        for (const SupportPoint &firstSupportPoint : supportPoints)
        {
            if (gravityFit)
            {
                break;
            };

            for (const SupportPoint &secondSupportPoint : supportPoints)
            {
                if (gravityFit)
                {
                    break;
                };

                // No need to check if supporting points are compatible if:
                // 1. They are the same, i.e. same parent item.
                // 2. They are not supporting along the same axis.
                if (firstSupportPoint.parentItem_ == secondSupportPoint.parentItem_ ||
                    firstSupportPoint.supportingAxis_ != secondSupportPoint.supportingAxis_)
                {
                    continue;
                };

                const int totalDistance = (firstSupportPoint.getOppositeAxis() == constants::axis::WIDTH ? aItemBeingPlaced->width_ : aItemBeingPlaced->depth_);
                gravityFit = (firstSupportPoint.supportDistance(secondSupportPoint) / totalDistance * 100) >= DEFAULT_VERTICE_DISTANCE_THRESHOLD;
            }
        };

        return gravityFit;
    };

public:
    bool highLevelGravityEnabled_;

    Gravity(int aGravityStrengthPercentage,
            std::shared_ptr<ItemRegister> &aItemRegister) : gravityStrength_(aGravityStrengthPercentage),
                                                            itemRegister_(aItemRegister)
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
     * @brief Checks if the item obeys gravity constraints.
     *
     * Several checks are executed:
     *
     * 1. Is gravity used?
     * 2. Is the item placed on the ground?
     * 3. Does the item have enough total ground support, or is the item supported under enough vertices?
     *
     * @param aItemBeingPlaced
     * @param aItemsInBin
     * @return true
     * @return false
     */
    const bool itemObeysGravity(const std::shared_ptr<Item> &aItemBeingPlaced,
                                const std::vector<int> &aItemsInBin) const
    {

        // If gravity is off, item is obeying gravity.
        if (!Gravity::gravityEnabled(aItemBeingPlaced))
        {
            return OBEYS_GRAVITY;
        }

        // If an item is placed on the ground, the item is obeying gravity.
        if (Gravity::itemIsPlacedOnGround(aItemBeingPlaced))
        {
            return OBEYS_GRAVITY;
        };

        const std::vector<int> supportingItems = Gravity::collectSupportingItems(aItemBeingPlaced, aItemsInBin);

        // If there are no supporting items, the item can not obey gravity.
        // Return false.
        if (supportingItems.empty())
        {
            return !OBEYS_GRAVITY;
        }

        // Evaluate the different types of gravity support.
        return Gravity::hasSurfaceSupport(aItemBeingPlaced, supportingItems) ||
               Gravity::hasVerticeSupport(aItemBeingPlaced, supportingItems);
    }
};

#endif