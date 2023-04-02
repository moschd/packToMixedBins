#ifndef GEOMETRIC_SHAPE_2D_H
#define GEOMETRIC_SHAPE_2D_H

class GeometricShape2D

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
    double width_;
    double depth_;
    double height_;
    double volume_;
    double original_width_;
    double original_depth_;
    double original_height_;
    std::array<double, 3> position_;
    int rotationType_;
    std::string rotationTypeDescription_;
    double furthestPointWidth_;
    double furthestPointDepth_;
    double furthestPointHeight_;

    GeometricShape2D(double aWidth = 0.0,
                   double aDepth = 0.0,
                   double aHeight = 0.0) : width_(aWidth),
                                           depth_(aDepth),
                                           height_(aHeight),
                                           volume_(aWidth * aDepth * aHeight),
                                           original_width_(aWidth),
                                           original_depth_(aDepth),
                                           original_height_(aHeight),
                                           position_(constants::START_POSITION),
                                           rotationType_(constants::rotation::type::WDH),
                                           rotationTypeDescription_(""),
                                           furthestPointWidth_(0.0),
                                           furthestPointDepth_(0.0),
                                           furthestPointHeight_(0.0){};

    /**
     * @brief Set dimensions and rotation description based on current rotation type.
     *
     */
    void setNewDimensions()
    {
        switch (GeometricShape2D::rotationType_)
        {
        case constants::rotation::type::WDH:
            GeometricShape2D::width_ = GeometricShape2D::original_width_;
            GeometricShape2D::depth_ = GeometricShape2D::original_depth_;
            GeometricShape2D::height_ = GeometricShape2D::original_height_;
            GeometricShape2D::rotationTypeDescription_ = "No rotation";
            break;
        case constants::rotation::type::DWH:
            GeometricShape2D::width_ = GeometricShape2D::original_depth_;
            GeometricShape2D::depth_ = GeometricShape2D::original_width_;
            GeometricShape2D::height_ = GeometricShape2D::original_height_;
            GeometricShape2D::rotationTypeDescription_ = "Rotate around the z-axis by 90°";
            break;
        };
    };

    /**
     * @brief Determines the furthest point in space of this shape.
     *
     */
    inline void setFurthestPoints()
    {
        GeometricShape2D::furthestPointWidth_ = GeometricShape2D::position_[constants::axis::WIDTH] + GeometricShape2D::width_;
        GeometricShape2D::furthestPointDepth_ = GeometricShape2D::position_[constants::axis::DEPTH] + GeometricShape2D::depth_;
        GeometricShape2D::furthestPointHeight_ = GeometricShape2D::position_[constants::axis::HEIGHT] + GeometricShape2D::height_;
    };
};
#endif