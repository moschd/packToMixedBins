#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace Geometry
{

    template <class T>
    inline const bool intersectingX(const T cuboidOne, const T cuboidTwo)
    {
        return cuboidOne->furthestPointWidth_ > cuboidTwo->position_[constants::axis::WIDTH] &&
               cuboidOne->position_[constants::axis::WIDTH] < cuboidTwo->furthestPointWidth_;
    }

    template <class T>
    inline const bool intersectingY(const T cuboidOne, const T cuboidTwo)
    {
        return cuboidOne->furthestPointDepth_ > cuboidTwo->position_[constants::axis::DEPTH] &&
               cuboidOne->position_[constants::axis::DEPTH] < cuboidTwo->furthestPointDepth_;
    }

    template <class T>
    inline const bool intersectingZ(const T cuboidOne, const T cuboidTwo)
    {
        return cuboidOne->furthestPointHeight_ > cuboidTwo->position_[constants::axis::HEIGHT] &&
               cuboidOne->position_[constants::axis::HEIGHT] < cuboidTwo->furthestPointHeight_;
    }

    template <class T>
    inline const bool intersectingXY(const T cuboidOne, const T cuboidTwo)
    {
        return intersectingX(cuboidOne, cuboidTwo) && intersectingY(cuboidOne, cuboidTwo);
    }

    template <class T>
    inline const double nearestBoundary(const T objectOne, const T objectTwo)
    {
        return std::min(std::min(objectOne->position_[constants::axis::WIDTH] - objectTwo->position_[constants::axis::WIDTH],
                                 objectOne->position_[constants::axis::DEPTH] - objectTwo->position_[constants::axis::DEPTH]),
                        objectOne->position_[constants::axis::HEIGHT] - objectTwo->position_[constants::axis::HEIGHT]);
    }

    template <class T>
    inline const bool xIntersectCylinderCuboid(const T cylinder, const T cuboid)
    {
        double closestX = std::clamp(cylinder->position_[constants::axis::WIDTH], cuboid->position_[constants::axis::WIDTH], cuboid->furthestPointWidth_);
        return cylinder->radius_ < (cylinder->position_[constants::axis::WIDTH] - closestX);
    };

    template <class T>
    inline const bool yIntersectCylinderCuboid(const T cylinder, const T cuboid)
    {
        double closestY = std::clamp(cylinder->position[constants::axis::DEPTH], cuboid->position_[constants::axis::DEPTH], cuboid->furthestPointDepth_);
        return cylinder->radius_ < (cylinder->position[constants::axis::DEPTH] - closestY);
    };

    template <class T>
    inline const bool xyIntersectCylinderCuboid(const T cylinder, const T cuboid)
    {
        return intersectingX(cylinder, cuboid) && intersectingY(cylinder, cuboid);
    }
}
#endif