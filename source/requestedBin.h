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
    double maxWidth_;
    double maxDepth_;
    double maxHeight_;
    double maxVolume_;
    double maxWeight_;
    double estAvgVolumeUtil_;
    double estAvgWeightUtil_;
    std::array<int, 3> packingDirection_;

    /**
     * @brief Set the estimated number of bin values for the weight and volume dimension.
     *
     * @param aItemsToBePacked
     */
    void setEstAvgUtilPerBin(const std::vector<int> aItemsToBePacked, ItemRegister &aItemRegister)
    {
        double totalItemVolume;
        double totalItemWeight;

        for (auto itemKey : aItemsToBePacked)
        {
            const Item *itemToCheck = &aItemRegister.ItemRegister::getConstItem(itemKey);

            totalItemVolume += itemToCheck->volume_;
            totalItemWeight += itemToCheck->weight_;
        };

        const unsigned int estNrOfBins = std::max(std::ceil(totalItemVolume / RequestedBin::maxVolume_),
                                                  std::ceil(totalItemWeight / RequestedBin::maxWeight_));

        RequestedBin::estAvgVolumeUtil_ = totalItemVolume / (estNrOfBins * RequestedBin::maxVolume_) * 100;
        RequestedBin::estAvgWeightUtil_ = totalItemWeight / (estNrOfBins * RequestedBin::maxWeight_) * 100;
    };

    /**
     * @brief Set the desired packing direction of the bin.
     *
     * @param aDirection
     */
    void setPackingDirection(std::string aDirection)
    {

        std::transform(aDirection.begin(), aDirection.end(), aDirection.begin(), ::toupper);

        if (aDirection == constants::bin::parameter::BACK_TO_FRONT)
        {
            RequestedBin::packingDirection_ = {constants::axis::WIDTH, constants::axis::HEIGHT, constants::axis::DEPTH};
        }
        else
        {
            RequestedBin::packingDirection_ = {constants::axis::WIDTH, constants::axis::DEPTH, constants::axis::HEIGHT};
        };
    }

public:
    RequestedBin(std::string aBinType,
                 double aBinWidth,
                 double aBinDepth,
                 double aBinHeight,
                 double aBinMaxWeight,
                 std::string aPackingDirection) : type_(aBinType),
                                                  maxWidth_(aBinWidth),
                                                  maxDepth_(aBinDepth),
                                                  maxHeight_(aBinHeight),
                                                  maxWeight_(aBinMaxWeight)
    {
        RequestedBin::maxVolume_ = (RequestedBin::maxWidth_ * RequestedBin::maxDepth_ * RequestedBin::maxHeight_);
        RequestedBin::setPackingDirection(aPackingDirection);
    };

    /**
     * @brief Get the requested bin type.
     *
     * @return const std::string
     */
    const std::string getType() const
    {
        return RequestedBin::type_;
    };

    /**
     * @brief Get the requested width.
     *
     * @return const double
     */
    const double getWidth() const
    {
        return RequestedBin::maxWidth_;
    };

    /**
     * @brief Get the requested depth.
     *
     * @return const double
     */
    const double getDepth() const
    {
        return RequestedBin::maxDepth_;
    };

    /**
     * @brief Get the requested height.
     *
     * @return const double
     */
    const double getHeight() const
    {
        return RequestedBin::maxHeight_;
    };

    /**
     * @brief Get the requested maxWeight.
     *
     * @return const double
     */
    const double getMaxWeight() const
    {
        return RequestedBin::maxWeight_;
    };

    /**
     * @brief Get the requested maxVolume.
     *
     * @return const double
     */
    const double getMaxVolume() const
    {
        return RequestedBin::maxVolume_;
    };

    /**
     * @brief Public helper function to set the estimated numbers.
     *
     * @param aItemsToBePacked
     * @param aItemRegister
     */
    void setEstimatedAverages(const std::vector<int> aItemsToBePacked, ItemRegister &aItemRegister)
    {
        RequestedBin::setEstAvgUtilPerBin(aItemsToBePacked, aItemRegister);
    };

    /**
     * @brief Get the estimated average volume utilization per bin.
     *
     * @return const double
     */
    const double getEstAvgVolumeUtil() const
    {
        return RequestedBin::estAvgVolumeUtil_;
    }

    /**
     * @brief Get the estimated average weight utilization per bin.
     *
     * @return const double
     */
    const double getEstAvgWeightUtil() const
    {
        return RequestedBin::estAvgWeightUtil_;
    }

    /**
     * @brief Get the desired packing direction of the bin.
     *
     * @return const std::array<int, 3>
     */
    const std::array<int, 3> getPackingDirection() const
    {
        return RequestedBin::packingDirection_;
    }
};

#endif