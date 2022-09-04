#ifndef RECTANGULAR_CUBOID_H
#define RECTANGULAR_CUBOID_H

class RectangularCuboid
{
public:
    double width_;
    double depth_;
    double height_;
    double maxVolume_;
    std::array<double, 3> topRightCorner_;
    double smallestDimension_;
    double furthestPointWidth_;
    double furthestPointDepth_;
    double furthestPointHeight_;

    RectangularCuboid(double aWidth, double aDepth, double aHeight) : width_(aWidth),
                                                                      depth_(aDepth),
                                                                      height_(aHeight)
    {
        maxVolume_ = width_ * depth_ * height_;
        topRightCorner_ = {width_, depth_, height_};
        smallestDimension_ = std::min(std::min(width_, depth_), height_);
    };
};

#endif