#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace Geometry
{

    template <class T>
    inline bool intersectingX(T itemOne, T itemTwo)
    {
        return itemOne->furthestPointWidth_ > itemTwo->position_[constants::axis::WIDTH] &&
               itemOne->position_[constants::axis::WIDTH] < itemTwo->furthestPointWidth_;
    }

    template <class T>
    inline bool intersectingY(T itemOne, T itemTwo)
    {
        return itemOne->furthestPointDepth_ > itemTwo->position_[constants::axis::DEPTH] &&
               itemOne->position_[constants::axis::DEPTH] < itemTwo->furthestPointDepth_;
    }
    template <class T>
    inline bool intersectingZ(T itemOne, T itemTwo)
    {
        return itemOne->furthestPointHeight_ > itemTwo->position_[constants::axis::HEIGHT] &&
               itemOne->position_[constants::axis::HEIGHT] < itemTwo->furthestPointHeight_;
    }

    template <class T>
    inline bool intersectingXY(T itemOne, T itemTwo)
    {
        return intersectingX(itemOne, itemTwo) && intersectingY(itemOne, itemTwo);
    }

    template <class T>
    inline double nearestBoundary(T objectOne, T objectTwo)
    {
        return std::min(std::min(objectOne->position_[constants::axis::WIDTH] - objectTwo->position_[constants::axis::WIDTH],
                                 objectOne->position_[constants::axis::DEPTH] - objectTwo->position_[constants::axis::DEPTH]),
                        objectOne->position_[constants::axis::HEIGHT] - objectTwo->position_[constants::axis::HEIGHT]);
    }
}
#endif