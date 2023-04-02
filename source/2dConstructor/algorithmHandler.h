#ifndef ALGORITHM_HANDLER_H
#define ALGORITHM_HANDLER_H

/**
 * This class handles the algorithm evaluation/iteration.
 * It provides an interface between the different algorithms and the bin packing.
 */
class AlgorithmHandler
{
private:
    int binWidth_;
    int binHeight_;
    int rectWidth_;
    int rectHeight_;
    bool allowRotation_;
    std::unique_ptr<MaxRectsBinPack> maxRectsPackingInstance_;
    std::unique_ptr<NeatPacker> neatPackingInstance_;
    std::unique_ptr<SpiralPacker> spiralPackingInstance_;

    const Rect emptyNode() const
    {
        Rect emptyNode = {};
        return emptyNode;
    }

public:
    AlgorithmHandler(const int aBinWidth,
                     const int aBinHeight,
                     const int aRectWidth,
                     const int aRectHeight,
                     const bool aAllowRotation) : binWidth_(aBinWidth),
                                                  binHeight_(aBinHeight),
                                                  rectWidth_(aRectWidth),
                                                  rectHeight_(aRectHeight),
                                                  allowRotation_(aAllowRotation)
    {
        AlgorithmHandler::maxRectsPackingInstance_ = std::make_unique<MaxRectsBinPack>(binWidth_, binHeight_, rectWidth_, rectHeight_, allowRotation_);
        AlgorithmHandler::neatPackingInstance_ = std::make_unique<NeatPacker>(binWidth_, binHeight_, rectWidth_, rectHeight_);
        AlgorithmHandler::spiralPackingInstance_ = std::make_unique<SpiralPacker>(binWidth_, binHeight_, rectWidth_, rectHeight_, allowRotation_);
    };

    Rect placeItem(HeuristicAlgorithmType aAlgorithm)
    {
        Rect newNode;

        switch (aAlgorithm)
        {
        case RectBestShortSideFit:
        case RectBottomLeftRule:
        case RectContactPointRule:
        case RectBestLongSideFit:
        case RectBestAreaFit:
            newNode = maxRectsPackingInstance_->insert(aAlgorithm);
            break;
        // case SpiralFitPacker:
        //     newNode = spiralPackingInstance_->insert();
        //     break;
        case NeatGroupFitPacker:
            newNode = neatPackingInstance_->insert();
            break;
        case Last:
            // This one is not doing anything and should always be last.
            break;
        }

        // Safety check...
        bool exceedsBin = (newNode.x + newNode.width > binWidth_) || (newNode.y + newNode.height > binHeight_);
        if (exceedsBin)
        {
            printf("\n!!! Safety break triggered !!!\n");
            printf("--- Check algorithm %d, this must not happen.\n\n", aAlgorithm);
            printf("--- Bin width: %d, height: %d.\n", binWidth_, binHeight_);
            printf("--- Item x: %d, y: %d, width: %d, height: %d.\n", newNode.x, newNode.y, newNode.width, newNode.height);
            newNode = AlgorithmHandler::emptyNode();
        }

        return newNode;
    };

    void cleanAlgorithmHistory()
    {
        AlgorithmHandler::maxRectsPackingInstance_->reset();

        /// These do not need to be cleaned since they only occur once.
        // AlgorithmHandler::spiralPackingInstance_->reset();
        // AlgorithmHandler::neatPackingInstance_->reset();
        // AlgorithmHandler::diagonalPackingInstance_->reset();
    }
};

#endif