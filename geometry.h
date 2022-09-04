#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace Geometry
{

    template <class T>
    inline const bool intersectingX(const T itemOne, const T itemTwo)
    {
        return itemOne->furthestPointWidth_ > itemTwo->position_[constants::axis::WIDTH] &&
               itemOne->position_[constants::axis::WIDTH] < itemTwo->furthestPointWidth_;
    }

    template <class T>
    inline const bool intersectingY(const T itemOne, const T itemTwo)
    {
        return itemOne->furthestPointDepth_ > itemTwo->position_[constants::axis::DEPTH] &&
               itemOne->position_[constants::axis::DEPTH] < itemTwo->furthestPointDepth_;
    }
    template <class T>
    inline const bool intersectingZ(const T itemOne, const T itemTwo)
    {
        return itemOne->furthestPointHeight_ > itemTwo->position_[constants::axis::HEIGHT] &&
               itemOne->position_[constants::axis::HEIGHT] < itemTwo->furthestPointHeight_;
    }

    template <class T>
    inline const bool intersectingXY(const T itemOne, const T itemTwo)
    {
        return intersectingX(itemOne, itemTwo) && intersectingY(itemOne, itemTwo);
    }

    template <class T>
    inline const double nearestBoundary(const T objectOne, const T objectTwo)
    {
        return std::min(std::min(objectOne->position_[constants::axis::WIDTH] - objectTwo->position_[constants::axis::WIDTH],
                                 objectOne->position_[constants::axis::DEPTH] - objectTwo->position_[constants::axis::DEPTH]),
                        objectOne->position_[constants::axis::HEIGHT] - objectTwo->position_[constants::axis::HEIGHT]);
    }
}
#endif