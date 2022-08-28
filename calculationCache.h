#ifndef CALCULATION_CACHE_H
#define CALCULATION_CACHE_H
/**
 * @brief Cache manager to help with space management and speed up calculation.
 *
 *
 */
class CalculationCache
{
public:
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

    // std::unordered_map<std::array<double, 3>, double, itemPositionHash> intersectionLimits;
    __gnu_pbds::gp_hash_table<std::array<double, 3>, double, itemPositionHash> intersectionLimits;

    CalculationCache(){};

    /**
     * @brief Adds item to intersect cache.
     *
     * The shortest distance to the intersecting item gets stored.
     * This later gets compared to the smallest dimension of an item to be placed on these coordinates.
     *
     * @param aItemBeingPlaced
     * @param aItemAlreadyPlaced
     */
    void addIntersection(const Item *aItemBeingPlaced, const Item *aItemAlreadyPlaced)
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
     * @param aItemToBeFound
     * @return true
     * @return false
     */
    bool itemPositionCacheHit(const Item *aItemToBeFound)
    {
        bool hit = 0;
        const auto resultIterator = CalculationCache::intersectionLimits.find(aItemToBeFound->Item::position_);
        if (resultIterator != CalculationCache::intersectionLimits.end())
        {
            hit = aItemToBeFound->smallestDimension_ >= resultIterator->second;
        };
        return hit;
    };
};
#endif