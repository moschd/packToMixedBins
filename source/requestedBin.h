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
    int nrOfAvailableBins_;
    int itemLimit_;
    double maxVolume_;
    double maxWeight_;
    double estAvgVolumeUtil_;
    double estAvgWeightUtil_;
    std::array<int, 3> packingDirection_;
    std::string itemSortMethod_;
    double binGravityStrength_;

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
    };

public:
    RequestedBin(std::string aBinType,
                 int aBinWidth,
                 int aBinDepth,
                 int aBinHeight,
                 double aBinMaxWeight,
                 int aNrOfAvailableBins,
                 int aItemLimit,
                 std::string aPackingDirection,
                 std::string aItemSortMethod,
                 double aBinGravityStrength) : type_(aBinType),
                                               maxWidth_(aBinWidth),
                                               maxDepth_(aBinDepth),
                                               maxHeight_(aBinHeight),
                                               maxWeight_(aBinMaxWeight),
                                               nrOfAvailableBins_(aNrOfAvailableBins),
                                               itemLimit_(aItemLimit),
                                               itemSortMethod_(aItemSortMethod),
                                               binGravityStrength_(aBinGravityStrength)
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
    const std::string &getItemSortMethod() const { return RequestedBin::itemSortMethod_; }
    const double &getBinGravityStrength() const { return RequestedBin::binGravityStrength_; }

    // Set to -1 if not applicable.
    const int getItemLimit() const { return RequestedBin::itemLimit_; };

    /**
     * @brief Compares integer to the available number of bins as specified by the input.
     *
     * Default value of this attribute is 0, so if availableBins is 0 it means the user did not provide any limitation.
     *
     * @param aInteger
     * @return true
     * @return false
     */
    const bool exceedsNrOfAvailableBins(const int aInteger) const
    {

        // Default value is 0, so if 0, we do not enforce any limit on the availableBins.
        if (RequestedBin::nrOfAvailableBins_ == 0)
        {
            return false;
        }

        return RequestedBin::nrOfAvailableBins_ < aInteger;
    }

    /**
     * @brief Compares integer to the allowed number of items in the bin as specified by the input.
     *
     * Default value of this attribute is 0, so if itemLimit is 0 it means the user did not provide any limitation.
     *
     * @param aInteger
     * @return true
     * @return false
     */
    const bool exceedsItemLimit(const int aInteger) const
    {
        if (RequestedBin::itemLimit_ == 0)
        {
            return false;
        }

        return RequestedBin::itemLimit_ < aInteger;
    }
};

#endif