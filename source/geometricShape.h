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
    double width_;
    double depth_;
    double height_;
    double original_width_;
    double original_height_;
    double original_depth_;
    double smallestDimension_;
    double furthestPointWidth_;
    double furthestPointDepth_;
    double furthestPointHeight_;
    std::array<double, 3> position_;
    double volume_;
    int rotationType_;
    std::string allowedRotations_;
    std::string rotationTypeDescription_;

    GeometricShape(double aWidth = 0.0,
                   double aDepth = 0.0,
                   double aHeight = 0.0,
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
        GeometricShape::volume_ = width_ * depth_ * height_;
        GeometricShape::smallestDimension_ = std::min(std::min(GeometricShape::width_, GeometricShape::depth_), GeometricShape::height_);
        GeometricShape::allowedRotations_ = GeometricShape::allowedRotations_.size() == 0 ? "012345" : GeometricShape::allowedRotations_;
    };

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