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
    inline const int nearestBoundary(const T &objectOne, const T &objectTwo)
    {

        const int widthDistance = std::max(objectOne->position_[constants::axis::WIDTH], objectTwo->position_[constants::axis::WIDTH]) -
                                  std::min(objectOne->position_[constants::axis::WIDTH], objectTwo->position_[constants::axis::WIDTH]);

        const int depthDistance = std::max(objectOne->position_[constants::axis::DEPTH], objectTwo->position_[constants::axis::DEPTH]) -
                                  std::min(objectOne->position_[constants::axis::DEPTH], objectTwo->position_[constants::axis::DEPTH]);

        const int heightDistance = std::max(objectOne->position_[constants::axis::HEIGHT], objectTwo->position_[constants::axis::HEIGHT]) -
                                   std::min(objectOne->position_[constants::axis::HEIGHT], objectTwo->position_[constants::axis::HEIGHT]);

        std::vector<int> distances = {widthDistance, depthDistance, heightDistance};
        std::sort(distances.begin(), distances.end());

        for (int distance : distances)
        {
            // if (objectOne->id_ == "DTM002" && objectTwo->id_ == "DTM012")
            // {
            //     std::cout << " distance of " << distance << "\n";
            // }
            if (distance > 0)
            {

                // if (objectOne->id_ == "DTM002" && objectTwo->id_ == "DTM012")
                // {
                //     std::cout << "Returning smallest distance of " << distance << "\n";
                // }
                return distance;
            };
        };

        return 0;
    }
}
#endif