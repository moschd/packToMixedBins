#ifndef GEOMETRIC_SHAPE_H
#define GEOMETRIC_SHAPE_H

class GeometricShape

/**
 * @brief Class that contains all attributes for geometric shapes.
 *
 * Currently supports:
 * - Cuboid
 *
 */
{

private:
public:
    std::string shape_;
    int width_;
    int depth_;
    int height_;
    int original_width_;
    int original_height_;
    int original_depth_;
    int smallestDimension_;
    int furthestPointWidth_;
    int furthestPointDepth_;
    int furthestPointHeight_;
    std::array<int, 3> position_;
    double volume_;
    int rotationType_;
    std::string allowedRotations_;
    std::string rotationTypeDescription_;

    GeometricShape(int aWidth = 0,
                   int aDepth = 0,
                   int aHeight = 0,
                   std::string aAllowedRotations = "") : width_(aWidth),
                                                         depth_(aDepth),
                                                         height_(aHeight),
                                                         original_width_(aWidth),
                                                         original_depth_(aDepth),
                                                         original_height_(aHeight),
                                                         furthestPointWidth_(0),
                                                         furthestPointDepth_(0),
                                                         furthestPointHeight_(0),
                                                         position_(constants::START_POSITION),
                                                         allowedRotations_(aAllowedRotations),
                                                         rotationType_(constants::rotation::type::WDH),
                                                         rotationTypeDescription_("")
    {
        GeometricShape::shape_ = constants::shape::CUBOID;
        GeometricShape::smallestDimension_ = std::min(std::min(GeometricShape::width_, GeometricShape::depth_), GeometricShape::height_);
        GeometricShape::allowedRotations_ = GeometricShape::allowedRotations_.size() == 0 ? "012345" : GeometricShape::allowedRotations_;

        GeometricShape::volume_ = ((double)width_ / MULTIPLIER) * ((double)depth_ / MULTIPLIER) * ((double)height_ / MULTIPLIER);
    };

    const double getRealVolume() const { return volume_; };

    const double getRealWidth() const { return (double)width_ / MULTIPLIER; };
    const double getRealDepth() const { return (double)depth_ / MULTIPLIER; };
    const double getRealHeight() const { return (double)height_ / MULTIPLIER; };

    const double getRealBottomSurfaceArea() const { return getRealWidth() * getRealDepth(); };

    const double getRealOriginalWidth() const { return (double)original_width_ / MULTIPLIER; };
    const double getRealOriginalDepth() const { return (double)original_depth_ / MULTIPLIER; };
    const double getRealOriginalHeight() const { return (double)original_height_ / MULTIPLIER; };

    const double getRealFurthestPointWidth() const { return (double)furthestPointWidth_ / MULTIPLIER; };
    const double getRealFurthestPointDepth() const { return (double)furthestPointDepth_ / MULTIPLIER; };
    const double getRealFurthestPointHeight() const { return (double)furthestPointHeight_ / MULTIPLIER; };

    const double getRealXPosition() const { return (double)position_[constants::axis::WIDTH] / MULTIPLIER; };
    const double getRealYPosition() const { return (double)position_[constants::axis::DEPTH] / MULTIPLIER; };
    const double getRealZPosition() const { return (double)position_[constants::axis::HEIGHT] / MULTIPLIER; };

    /**
     * @brief Set dimensions and rotation description based on current rotation type.
     *
     */
    void setNewDimensions()
    {
        switch (GeometricShape::rotationType_)
        {
        case constants::rotation::type::WDH:
            GeometricShape::width_ = GeometricShape::original_width_;
            GeometricShape::depth_ = GeometricShape::original_depth_;
            GeometricShape::height_ = GeometricShape::original_height_;
            GeometricShape::rotationTypeDescription_ = "No rotation";
            break;
        case constants::rotation::type::DWH:
            GeometricShape::width_ = GeometricShape::original_depth_;
            GeometricShape::depth_ = GeometricShape::original_width_;
            GeometricShape::height_ = GeometricShape::original_height_;
            GeometricShape::rotationTypeDescription_ = "Rotate around the z-axis by 90°";
            break;
        case constants::rotation::type::HDW:
            GeometricShape::width_ = GeometricShape::original_height_;
            GeometricShape::depth_ = GeometricShape::original_depth_;
            GeometricShape::height_ = GeometricShape::original_width_;
            GeometricShape::rotationTypeDescription_ = "Rotate around the x-axis by 90°";
            break;
        case constants::rotation::type::DHW:
            GeometricShape::width_ = GeometricShape::original_depth_;
            GeometricShape::depth_ = GeometricShape::original_height_;
            GeometricShape::height_ = GeometricShape::original_width_;
            GeometricShape::rotationTypeDescription_ = "Rotate around the x-axis by 90° and then around the z-axis by 90°";
            break;
        case constants::rotation::type::HWD:
            GeometricShape::width_ = GeometricShape::original_height_;
            GeometricShape::depth_ = GeometricShape::original_width_;
            GeometricShape::height_ = GeometricShape::original_depth_;
            GeometricShape::rotationTypeDescription_ = "Rotate around the z-axis by 90° and then around the x-axis by 90°";
            break;
        case constants::rotation::type::WHD:
            GeometricShape::width_ = GeometricShape::original_width_;
            GeometricShape::depth_ = GeometricShape::original_height_;
            GeometricShape::height_ = GeometricShape::original_depth_;
            GeometricShape::rotationTypeDescription_ = "Rotate around the y-axis by 90°";
            break;
        };
    };

    /**
     * @brief Determines the furthest point in space of this shape.
     *
     */
    inline void setFurthestPoints()
    {
        GeometricShape::furthestPointWidth_ = GeometricShape::position_[constants::axis::WIDTH] + GeometricShape::width_;
        GeometricShape::furthestPointDepth_ = GeometricShape::position_[constants::axis::DEPTH] + GeometricShape::depth_;
        GeometricShape::furthestPointHeight_ = GeometricShape::position_[constants::axis::HEIGHT] + GeometricShape::height_;
    };
};
#endif