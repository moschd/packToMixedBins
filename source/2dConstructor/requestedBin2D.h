#ifndef REQUESTED_BIN_2D_H
#define REQUESTED_BIN_2D_H

/**
 * @brief Represents the requested bin.
 *
 * Contains attributes related to the bin that was requested to be used during packing.
 *
 */
class RequestedBin2D
{
private:
    std::string type_;
    double maxWidth_;
    double maxDepth_;
    double maxHeight_;
    double maxWeight_;
    double maxVolume_;
    double packingMarginWidth_;
    double packingMarginDepth_;
    double packingMarginHeight_;

public:
    RequestedBin2D(std::string aBinType,
                     double aBinWidth,
                     double aBinDepth,
                     double aBinHeight,
                     double aBinMaxWeight,
                     double aPackingMarginWidth,
                     double aPackingMarginDepth,
                     double aPackingMarginHeight) : type_(aBinType),
                                                    maxWidth_(aBinWidth),
                                                    maxDepth_(aBinDepth),
                                                    maxHeight_(aBinHeight),
                                                    maxWeight_(aBinMaxWeight),
                                                    maxVolume_(0),
                                                    packingMarginWidth_(aPackingMarginWidth),
                                                    packingMarginDepth_(aPackingMarginDepth),
                                                    packingMarginHeight_(aPackingMarginHeight)
    {
        RequestedBin2D::maxVolume_ = ((RequestedBin2D::maxWidth_ + RequestedBin2D::packingMarginWidth_) *
                                        (RequestedBin2D::maxDepth_ + RequestedBin2D::packingMarginDepth_) *
                                        (RequestedBin2D::maxHeight_ + RequestedBin2D::packingMarginHeight_));
    };

    /// @brief Get the type of the requested bin.
    /// @return const std::string
    const std::string getType() const { return RequestedBin2D::type_; };

    /// @brief Get the width of the requested bin.
    /// @return const double
    const double getWidth() const { return RequestedBin2D::maxWidth_; };

    /// @brief Get the depth of the requested bin.
    /// @return const double
    const double getDepth() const { return RequestedBin2D::maxDepth_; };

    /// @brief Get the height of the requested bin.
    /// @return const double
    const double getHeight() const { return RequestedBin2D::maxHeight_; };

    /// @brief Get the maximum weight of the requested bin.
    /// @return const double
    const double getMaxWeight() const { return RequestedBin2D::maxWeight_; };

    /// @brief Get the maximum volume of the requested bin.
    /// @return const double
    const double getMaxVolume() const { return RequestedBin2D::maxVolume_; }

    /// @brief Get the maximum distance on the x axis, this takes any allowed margin into account.
    /// @return const double
    const double getMaxAllowedPackingWidth() const { return RequestedBin2D::maxWidth_ + RequestedBin2D::packingMarginWidth_; }

    /// @brief Get the maximum distance on the y axis, this takes any allowed margin into account.
    /// @return const double
    const double getMaxAllowedPackingDepth() const { return RequestedBin2D::maxDepth_ + RequestedBin2D::packingMarginDepth_; }

    /// @brief Get the maximum distance on the z axis, this takes any allowed margin into account.
    /// @return const double
    const double getMaxAllowedPackingHeight() const { return RequestedBin2D::maxHeight_ + RequestedBin2D::packingMarginHeight_; }

    /// @brief Get allowed packing margin for x axis.
    /// @return const double
    const double getPackingMarginWidth() const { return RequestedBin2D::packingMarginWidth_; }

    /// @brief Get allowed packing margin for y axis.
    /// @return const double
    const double getPackingMarginDepth() const { return RequestedBin2D::packingMarginDepth_; }

    /// @brief Get allowed packing margin for z axis.
    /// @return const double
    const double getPackingMarginHeight() const { return RequestedBin2D::packingMarginHeight_; }
};

#endif