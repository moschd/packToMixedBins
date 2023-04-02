#ifndef BIN_CALCULATION_CACHE_H
#define BIN_CALCULATION_CACHE_H

/**
 * @brief Cache manager to help with space management and speed up calculation.
 *
 *
 */
class BinCalculationCache
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
    __gnu_pbds::gp_hash_table<std::array<double, 3>, double, itemPositionHash> intersectionLimits_;

public:
    BinCalculationCache(){};
    /**
     * @brief Adds item to intersect cache.
     *
     * The shortest distance to the intersecting item gets stored.
     * This later gets compared to the smallest dimension of an item to be placed on these coordinates.
     *
     * @param aItemBeingPlaced
     * @param aItemAlreadyPlaced
     */
    inline void addIntersection(const std::shared_ptr<Item> aItemBeingPlaced, const std::shared_ptr<Item> aItemAlreadyPlaced)
    {
        const double smallestDistance = Geometry::nearestBoundary(aItemBeingPlaced, aItemAlreadyPlaced);
        const auto &resultIterator = BinCalculationCache::intersectionLimits_.find(aItemBeingPlaced->position_);

        if (resultIterator == intersectionLimits_.end())
        {
            BinCalculationCache::intersectionLimits_[aItemBeingPlaced->Item::position_] = smallestDistance;
        }
        else
        {
            BinCalculationCache::intersectionLimits_[aItemBeingPlaced->Item::position_] = std::min(resultIterator->second, smallestDistance);
        };
    };

    /**
     * @brief Checks if item is valid or if intersecting items have been found already.
     *
     * Compares the items smallest dimension to the known intersection distance for this position.
     * If the items smallest dimension is bigger, it will not matter how we rotate the item since it simply will never fit.
     *
     * Short circuit boolean return to prevent additional assignment and if statement.
     *
     * @param aItemToBeFound
     * @return true
     * @return false
     */
    inline const bool itemPositionCacheHit(const std::shared_ptr<Item> aItemToBeFound) const
    {
        const auto resultIterator = BinCalculationCache::intersectionLimits_.find(aItemToBeFound->Item::position_);
        return resultIterator != BinCalculationCache::intersectionLimits_.end() &&
               aItemToBeFound->smallestDimension_ >= resultIterator->second;
    };
};
#endif