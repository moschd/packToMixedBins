#ifndef BIN_2D_H
#define BIN_2D_H

class Bin2D : public GeometricShape
{
private:
    std::shared_ptr<PackingContext> context_;
    std::vector<std::shared_ptr<PackingLayer>> layers_;
    int actualVolumeUtil_;
    int actualWeightUtil_;
    int maxItemsWeightConstraint_;

    /**
     * @brief Create the base layer of the bin.
     *
     * Since we are packing only homogenous items, we can perform expensive 2D packing once, then copy the layer.
     *
     */
    void constructBase()
    {
        std::shared_ptr<PackingLayer> winningLayer = std::make_shared<PackingLayer>();

        const bool allowRotation = true;
        const int binWidth = Bin2D::context_->getRequestedBin()->getWidth();
        const int binHeight = Bin2D::context_->getRequestedBin()->getDepth();
        const int rectWidth = Bin2D::context_->getItem(BASE_ITEM_KEY)->width_;
        const int rectHeight = Bin2D::context_->getItem(BASE_ITEM_KEY)->depth_;

        std::unique_ptr<AlgorithmHandler> algorithmHandler = std::make_unique<AlgorithmHandler>(binWidth, binHeight, rectWidth, rectHeight, allowRotation);

        /// @brief Run the different algorithms and store the best one.
        for (int algoInteger = HeuristicAlgorithmType::RectBestShortSideFit; algoInteger != HeuristicAlgorithmType::Last; algoInteger++)
        {
            int numberOfItemsInBase = 0;
            HeuristicAlgorithmType currentAlgorithm = static_cast<HeuristicAlgorithmType>(algoInteger);

            // Clean history before starting a new packing.
            algorithmHandler->cleanAlgorithmHistory();

            // Initialize bin and create packing layer.
            std::unique_ptr<PackingLayer> baseLayer = std::make_unique<PackingLayer>(Bin2D::getNewLayerId(), Bin2D::context_, constants::START_POSITION);

            /// @brief Get item positions one-by-one according to current algorithm iteration.
            ///        Once item position has been found, place it into the packingLayer.
            ///        Max base layer items is introduced to prevent endless loop.
            while (numberOfItemsInBase <= 5000)
            {
                // Check weight constraint.
                const bool weightFit = numberOfItemsInBase < Bin2D::maxItemsWeightConstraint_;

                // Insert item according to current algorithm.
                Rect packedRect = algorithmHandler->placeItem(currentAlgorithm);

                // Test if rectangle fits by checking rectangle attribute and weight limitation.
                if (weightFit && packedRect.height > 0)
                {
                    numberOfItemsInBase += 1;
                    const int xLocation = (int)packedRect.x;
                    const int yLocation = (int)packedRect.y;
                    const bool isRotated = (packedRect.width != rectWidth);
                    baseLayer->addItem(xLocation, yLocation, isRotated);
                }
                else
                {
#if _DEBUG
                    printf("Algorithm %d resulted in %d items.\n", currentAlgorithm, int(baseLayer->getFittedItems().size()));
#endif
                    if ((int)winningLayer->getFittedItems().size() <= numberOfItemsInBase)
                    // if (currentAlgorithm == 6)
                    {
                        winningLayer = std::move(baseLayer);
                    }
                    break;
                }
            }
        }

        // Add the most efficiently packed layer. This is the base layer.
        Bin2D::addLayer(winningLayer);
    }

    /// @brief Update the furthest points of the bin.
    /// @param aItems
    void updatePlacedMaxItemPositions(const std::vector<int> aItems)
    {
        for (const auto &item : aItems)
        {
            const std::shared_ptr<Item> it = Bin2D::context_->getItem(item);
            Bin2D::furthestPointWidth_ = std::max(Bin2D::furthestPointWidth_, it->Item::furthestPointWidth_);
            Bin2D::furthestPointDepth_ = std::max(Bin2D::furthestPointDepth_, it->Item::furthestPointDepth_);
            Bin2D::furthestPointHeight_ = std::max(Bin2D::furthestPointHeight_, it->Item::furthestPointHeight_);
        }
    };

    /// @brief Add a packed layer onto the bin.
    /// @param aLayer
    void addLayer(std::shared_ptr<PackingLayer> aLayer)
    {
        Bin2D::layers_.push_back(aLayer);
        Bin2D::updateBinTotals();
        Bin2D::updatePlacedMaxItemPositions(aLayer->getFittedItems());
    }

    /// @brief Update Bin attributes based on the (number of) items currently in the bin.
    void updateBinTotals()
    {
        const int numberOfItemsInBin = int(Bin2D::getFittedItems().size());
        Bin2D::actualVolumeUtil_ = (numberOfItemsInBin * Bin2D::context_->getItem(BASE_ITEM_KEY)->volume_);
        Bin2D::actualWeightUtil_ = (numberOfItemsInBin * Bin2D::context_->getItem(BASE_ITEM_KEY)->weight_);
    }

public:
    Bin2D(std::shared_ptr<PackingContext> aContext) : context_(aContext),
                                                      layers_({}),
                                                      actualVolumeUtil_(0),
                                                      actualWeightUtil_(0),
                                                      GeometricShape(aContext->getRequestedBin()->getWidth(),
                                                                     aContext->getRequestedBin()->getDepth(),
                                                                     aContext->getRequestedBin()->getHeight())
    {
        Bin2D::layers_.reserve(10);
        Bin2D::maxItemsWeightConstraint_ = (int)std::floor(aContext->getRequestedBin()->getMaxWeight() / aContext->getItem(BASE_ITEM_KEY)->weight_);
    };

    /// @brief Get the packing context.
    /// @return PackingContext*
    const std::shared_ptr<PackingContext> getContext() const { return Bin2D::context_; }

    /// @brief Get all layers inside this bin.
    /// @return const std::vector<std::shared_ptr<PackingLayer>>
    const std::vector<std::shared_ptr<PackingLayer>> getLayers() const { return Bin2D::layers_; };

    /// @brief Get the actual volume utilization of this bin expressed in percentage points.
    /// @return const int
    const int getActVolumeUtilPercentage() const { return Bin2D::actualVolumeUtil_ / Bin2D::context_->getRequestedBin()->getMaxVolume() * 100; };

    /// @brief Get the actual weight utilization of this bin expressed in percentage points.
    /// @return const int
    const int getActWeightUtilPercentage() const { return Bin2D::actualWeightUtil_ / Bin2D::context_->getRequestedBin()->getMaxWeight() * 100; };

    /// @brief Get the actual weight utilization expressed in the weight units.
    /// @return const int
    const int getActWeightUtil() const { return Bin2D::actualWeightUtil_; };

    /// @brief Get the actual volume utilization expressed in the volume units.
    /// @return const int
    const int getActVolumeUtil() const { return Bin2D::actualVolumeUtil_; };

    /// @brief Generate a new id for a packing layer.
    /// @return const int
    const int getNewLayerId() const { return int(Bin2D::layers_.size() + 1); };

    /// @brief Get the base layer for this bin.
    /// @return const PackingLayer
    const std::shared_ptr<PackingLayer> getBaseLayer() const { return Bin2D::layers_.front(); }

    /// @brief Get the number of items that are packed per layer.
    /// @return const int
    const int getItemsPerLayer() const { return Bin2D::layers_.empty() ? 0 : int(Bin2D::getBaseLayer()->getFittedItems().size()); }

    /// @brief Get the 2d covered surface area for layer.
    /// @return const double
    const double getCoveredSurfaceArea() const
    {
        return (Bin2D::context_->getItem(BASE_ITEM_KEY)->getRealWidth() * Bin2D::context_->getItem(BASE_ITEM_KEY)->getRealDepth() * Bin2D::getItemsPerLayer()) /
               Bin2D::getRealBottomSurfaceArea() * 100;
    };

    /// @brief Aggregate items from layers and return them as a single vector.
    /// @return const std::vector<std::shared_ptr<Item>>
    const std::vector<std::shared_ptr<Item>> getFittedItems() const
    {
        std::vector<std::shared_ptr<Item>> myItems;
        for (const auto &itemKey : Bin2D::getBaseLayer()->getFittedItems())
        {
            myItems.push_back(Bin2D::context_->getItem(itemKey));
        }
        return myItems;
    };

    /// @brief Start packing the bin.
    void startPacking()
    {
        Bin2D::constructBase();
    }
};

#endif