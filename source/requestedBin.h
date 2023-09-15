#ifndef REQUESTED_BIN_H
#define REQUESTED_BIN_H

/**
 * @brief Represents the requested bin.
 *
 * Contains attributes related to the bin that was requested to be used during packing.
 *
 */
class RequestedBin
{
private:
    std::string type_;
    int maxWidth_;
    int maxDepth_;
    int maxHeight_;
    double maxVolume_;
    double maxWeight_;
    double estAvgVolumeUtil_;
    double estAvgWeightUtil_;
    std::array<int, 3> packingDirection_;

    /**
     * @brief Set the desired packing direction of the bin.
     *
     * @param aDirection
     */
    void setPackingDirection(std::string aDirection)
    {

        std::transform(aDirection.begin(), aDirection.end(), aDirection.begin(), ::toupper);

        if (aDirection == constants::bin::parameter::BACK_TO_FRONT_TEXT)
        {
            RequestedBin::packingDirection_ = constants::bin::parameter::BACK_TO_FRONT_ARRAY;
        }
        else
        {
            RequestedBin::packingDirection_ = constants::bin::parameter::BOTTOM_UP_ARRAY;
        };
    }

public:
    RequestedBin(std::string aBinType,
                 int aBinWidth,
                 int aBinDepth,
                 int aBinHeight,
                 double aBinMaxWeight,
                 std::string aPackingDirection) : type_(aBinType),
                                                  maxWidth_(aBinWidth),
                                                  maxDepth_(aBinDepth),
                                                  maxHeight_(aBinHeight),
                                                  maxWeight_(aBinMaxWeight)
    {
        RequestedBin::maxVolume_ = ((double)RequestedBin::maxWidth_ / MULTIPLIER) * ((double)RequestedBin::maxDepth_ / MULTIPLIER) * ((double)RequestedBin::maxHeight_ / MULTIPLIER);
        RequestedBin::setPackingDirection(aPackingDirection);
    };

    const std::string getType() const { return RequestedBin::type_; };

    const int getWidth() const { return RequestedBin::maxWidth_; };
    const int getDepth() const { return RequestedBin::maxDepth_; };
    const int getHeight() const { return RequestedBin::maxHeight_; };
    const double getMaxWeight() const { return RequestedBin::maxWeight_; };
    const double getMaxVolume() const { return RequestedBin::maxVolume_; };

    const double getRealWidth() const { return (double)maxWidth_ / MULTIPLIER; };
    const double getRealDepth() const { return (double)maxDepth_ / MULTIPLIER; };
    const double getRealBottomSurfaceArea() const { return getRealWidth() * getRealDepth(); };

    const std::array<int, 3> &getPackingDirection() const { return RequestedBin::packingDirection_; }

};

#endif