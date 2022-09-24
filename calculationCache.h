#ifndef CALCULATION_CACHE_H
#define CALCULATION_CACHE_H

/**
 * @brief Cache manager to help with space management and speed up calculation.
 *
 *
 */
class CalculationCache
{
private:
    struct itemPositionHash
    {
        size_t operator()(const std::array<double, 3> &itemPosition) const
        {
            size_t h1 = std::hash<double>()(itemPosition[constants::axis::WIDTH]);
            size_t h2 = std::hash<double>()(itemPosition[constants::axis::DEPTH]);
            size_t h3 = std::hash<double>()(itemPosition[constants::axis::HEIGHT]);
            return (h1 ^ (h2 << 1)) ^ (h3 << 2);
        }
    };
    __gnu_pbds::gp_hash_table<std::array<double, 3>, double, itemPositionHash> intersectionLimits;

public:
    /**
     * @brief Adds item to intersect cache.
     *
     * The shortest distance to the intersecting item gets stored.
     * This later gets compared to the smallest dimension of an item to be placed on these coordinates.
     *
     * @param aItemBeingPlaced
     * @param aItemAlreadyPlaced
     */
    inline void addIntersection(const Item *aItemBeingPlaced, const Item *aItemAlreadyPlaced)
    {
        const double smallestDistance = Geometry::nearestBoundary(aItemBeingPlaced, aItemAlreadyPlaced);
        const auto &resultIterator = CalculationCache::intersectionLimits.find(aItemBeingPlaced->position_);

        if (resultIterator == intersectionLimits.end())
        {
            CalculationCache::intersectionLimits[aItemBeingPlaced->Item::position_] = smallestDistance;
        }
        else
        {
            CalculationCache::intersectionLimits[aItemBeingPlaced->Item::position_] = std::min(resultIterator->second, smallestDistance);
        };
    };

    /**
     * @brief Checks if item is valid or if intersecting items have been found already.
     *
     * Short circuit boolean return to prevent additional assignment and if statement.
     *
     * @param aItemToBeFound
     * @return true
     * @return false
     */
    inline const bool itemPositionCacheHit(const Item *aItemToBeFound) const
    {
        const auto resultIterator = CalculationCache::intersectionLimits.find(aItemToBeFound->Item::position_);
        return resultIterator != CalculationCache::intersectionLimits.end() &&
               aItemToBeFound->smallestDimension_ >= resultIterator->second;
    };
};
#endif