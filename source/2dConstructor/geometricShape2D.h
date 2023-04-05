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
    int width_;
    int depth_;
    int height_;
    int volume_;
    int original_width_;
    int original_depth_;
    int original_height_;
    std::array<int, 3> position_;
    int rotationType_;
    std::string rotationTypeDescription_;
    int furthestPointWidth_;
    int furthestPointDepth_;
    int furthestPointHeight_;

    GeometricShape2D(int aWidth = 0,
                     int aDepth = 0,
                     int aHeight = 0) : width_(aWidth),
                                        depth_(aDepth),
                                        height_(aHeight),
                                        volume_(aWidth * aDepth * aHeight),
                                        original_width_(aWidth),
                                        original_depth_(aDepth),
                                        original_height_(aHeight),
                                        position_(constants::START_POSITION),
                                        rotationType_(constants::rotation::type::WDH),
                                        rotationTypeDescription_(""),
                                        furthestPointWidth_(0),
                                        furthestPointDepth_(0),
                                        furthestPointHeight_(0){};

    const double getRealWidth() const { return (double)width_ / MULTIPLIER; };
    const double getRealDepth() const { return (double)depth_ / MULTIPLIER; };
    const double getRealHeight() const { return (double)height_ / MULTIPLIER; };

    const double getReal2DSurfaceArea() const { return getRealWidth() * getRealDepth(); };

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