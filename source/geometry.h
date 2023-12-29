#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace Geometry
{

    template <class T>
    inline const bool intersectingX(const T &cuboidOne, const T &cuboidTwo)
    {
        return cuboidOne->furthestPointWidth_ > cuboidTwo->position_[constants::axis::WIDTH] &&
               cuboidOne->position_[constants::axis::WIDTH] < cuboidTwo->furthestPointWidth_;
    }

    template <class T>
    inline const bool intersectingY(const T &cuboidOne, const T &cuboidTwo)
    {
        return cuboidOne->furthestPointDepth_ > cuboidTwo->position_[constants::axis::DEPTH] &&
               cuboidOne->position_[constants::axis::DEPTH] < cuboidTwo->furthestPointDepth_;
    }

    template <class T>
    inline const bool intersectingZ(const T &cuboidOne, const T &cuboidTwo)
    {
        return cuboidOne->furthestPointHeight_ > cuboidTwo->position_[constants::axis::HEIGHT] &&
               cuboidOne->position_[constants::axis::HEIGHT] < cuboidTwo->furthestPointHeight_;
    }

    template <class T>
    inline const bool intersectingXY(const T &cuboidOne, const T &cuboidTwo)
    {
        return intersectingX(cuboidOne, cuboidTwo) && intersectingY(cuboidOne, cuboidTwo);
    }

/**
     * @brief Return the smallest distance between item starting points.
     * Ignores zeros unless all distances are 0.
     *
     * @tparam T
     * @param objectOne
     * @param objectTwo
     * @return const int
     */
    template <class T>
    inline const int nearestBoundary(const T &aItemBeingPlaced, const T &aItemAlreadyPlaced)
    {

        const int widthDistance = aItemAlreadyPlaced->position_[constants::axis::WIDTH] - aItemBeingPlaced->position_[constants::axis::WIDTH];
        const int depthDistance = aItemAlreadyPlaced->position_[constants::axis::DEPTH] - aItemBeingPlaced->position_[constants::axis::DEPTH];
        const int heightDistance = aItemAlreadyPlaced->position_[constants::axis::HEIGHT] - aItemBeingPlaced->position_[constants::axis::HEIGHT];

        std::vector<int> distances = {widthDistance, depthDistance, heightDistance};
        std::sort(distances.begin(), distances.end());

        // std::cout << aItemBeingPlaced->id_ << " and " << aItemAlreadyPlaced->id_ << "\n";
        // std::cout << "Width distance: " << widthDistance << "\n";
        // std::cout << "Depth distance: " << depthDistance << "\n";
        // std::cout << "Height distance: " << heightDistance << "\n";

        for (int distance : distances)
        {
            if (distance > 0)
            {
                return distance;
            };
        };

        return 0;
    }
}
#endif