#ifndef REQUESTED_BIN_HANDLER_H
#define REQUESTED_BIN_HANDLER_H

#include "requestedBin.h"

/**
 * @brief Helps managing the different requested bin objects.
 *
 */
class RequestedBinHandler
{
private:
    std::vector<std::shared_ptr<RequestedBin>> requestedBins_;
    int binIndex_;
    int nrOfBins_;

    void sortRequestedBins() {}

public:
    RequestedBinHandler() : binIndex_(0){};

    /**
     * @brief Function to add a bin.
     *
     * @param aRequestedBin
     */
    void addRequestedBin(std::shared_ptr<RequestedBin> aRequestedBin)
    {
        RequestedBinHandler::requestedBins_.push_back(aRequestedBin);
        RequestedBinHandler::nrOfBins_ = (int)RequestedBinHandler::requestedBins_.size();
    };

    /**
     * @brief Increment and rotate bin index.
     *
     */
    void rotateIndex()
    {
        std::cout << "Old bin index is " << RequestedBinHandler::binIndex_ << "\n";
        RequestedBinHandler::binIndex_ += 1;
        std::cout << "Incremented bin index is " << RequestedBinHandler::binIndex_ << "\n";
        std::cout << "Number of bins is " << RequestedBinHandler::nrOfBins_ << "\n";
        RequestedBinHandler::binIndex_ % RequestedBinHandler::nrOfBins_;
        std::cout << "Rotated bin index is " << RequestedBinHandler::binIndex_ << "\n";
    };

    /**
     * @brief Set the Bin Index to a particular value
     *
     * @param aIndex
     */
    void setBinIndex(const int aIndex) { binIndex_ = aIndex; };

    /**
     * @brief Get the requestedBin.
     *
     * @return const std::shared_ptr<RequestedBin>
     */
    const std::shared_ptr<RequestedBin> &getRequestedBin() const { return RequestedBinHandler::requestedBins_[RequestedBinHandler::binIndex_]; };
};

#endif