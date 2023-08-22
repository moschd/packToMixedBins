#ifndef NEAT_PACKER_H
#define NEAT_PACKER_H

class NeatPacker
{
private:
    int binWidth_;
    int binHeight_;
    int itemWidth_;
    int itemHeight_;
    std::array<int, 2> stackingDistance_;
    bool hasRecursed_;
    bool canRecurse_;
    int stackingAxis_;
    RectRow mostValuableRow_;
    std::array<RectRow, 2> rows_;
    std::vector<Rect> innerPackedItems_;
    std::unique_ptr<PackingAction> packingAction_;

    /// @brief Get the stacking distance relevant for this packing instance.
    /// @return const int
    const int getMyStackingDistance() const { return NeatPacker::stackingDistance_[NeatPacker::stackingAxis_]; };

    /// @brief Subtract a value from the available stacking distance.
    /// @param aNumberToSubtract
    void subtractMyStackingDistance(const int aNumberToSubtract) { NeatPacker::stackingDistance_[NeatPacker::stackingAxis_] -= aNumberToSubtract; };

    /// @brief Return the item width of the most valuable item.
    /// @return const int
    const int getMostValuableItemWidth() { return (NeatPacker::stackingAxis_ == constants::axis::WIDTH ? mostValuableRow_.width_ : mostValuableRow_.singularItemRowConsumption_); }

    /// @brief Return the item width of the most valuable item.
    /// @return const int
    const int getMostValuableItemHeight() { return (NeatPacker::stackingAxis_ == constants::axis::WIDTH ? mostValuableRow_.singularItemRowConsumption_ : mostValuableRow_.height_); }

    /// @brief Get a precomputed additional item from storage.
    /// @return const Rect
    const Rect consumePreComputedItem()
    {
        Rect newRect = NeatPacker::innerPackedItems_.front();
        NeatPacker::innerPackedItems_.erase(NeatPacker::innerPackedItems_.begin());
        return newRect;
    };

    /// @brief Return the secondary choice row.
    /// @return const RectRow
    const RectRow getSecondaryRow() const
    {
        return (mostValuableRow_.axis_ == constants::axis::WIDTH) ? rows_[constants::axis::DEPTH] : rows_[constants::axis::WIDTH];
    };

    /// @brief Initialize the rows. These are the rows that will be attempted to be placed inside the bin.
    /// The axis along which to stack has already been decided.
    void initializeRows()
    {
        RectRow xRotatedRow = {};
        RectRow yRotatedRow = {};

        xRotatedRow.axis_ = constants::axis::WIDTH;
        yRotatedRow.axis_ = constants::axis::DEPTH;

        if (NeatPacker::stackingAxis_ == constants::axis::DEPTH)
        {
            xRotatedRow.height_ = NeatPacker::itemHeight_;
            xRotatedRow.stackingSpaceConsumption_ = xRotatedRow.height_;
            xRotatedRow.singularItemRowConsumption_ = NeatPacker::itemWidth_;
            xRotatedRow.nrOfItems_ = (int)std::floor(NeatPacker::binWidth_ / xRotatedRow.singularItemRowConsumption_);
            xRotatedRow.width_ = xRotatedRow.nrOfItems_ * xRotatedRow.singularItemRowConsumption_;

            yRotatedRow.height_ = NeatPacker::itemWidth_;
            yRotatedRow.stackingSpaceConsumption_ = yRotatedRow.height_;
            yRotatedRow.singularItemRowConsumption_ = NeatPacker::itemHeight_;
            yRotatedRow.nrOfItems_ = (int)std::floor(NeatPacker::binWidth_ / yRotatedRow.singularItemRowConsumption_);
            yRotatedRow.width_ = yRotatedRow.nrOfItems_ * yRotatedRow.singularItemRowConsumption_;

            xRotatedRow.valid_ = (NeatPacker::binHeight_ >= xRotatedRow.height_ && NeatPacker::binWidth_ >= xRotatedRow.singularItemRowConsumption_);
            yRotatedRow.valid_ = (NeatPacker::binHeight_ >= yRotatedRow.height_ && NeatPacker::binWidth_ >= yRotatedRow.singularItemRowConsumption_);
        }
        else if (NeatPacker::stackingAxis_ == constants::axis::WIDTH)
        {
            xRotatedRow.width_ = NeatPacker::itemWidth_;
            xRotatedRow.stackingSpaceConsumption_ = NeatPacker::itemWidth_;
            xRotatedRow.singularItemRowConsumption_ = NeatPacker::itemHeight_;
            xRotatedRow.nrOfItems_ = (int)std::floor(NeatPacker::binHeight_ / NeatPacker::itemHeight_);
            xRotatedRow.height_ = xRotatedRow.nrOfItems_ * NeatPacker::itemHeight_;

            yRotatedRow.width_ = NeatPacker::itemHeight_;
            yRotatedRow.stackingSpaceConsumption_ = NeatPacker::itemHeight_;
            yRotatedRow.singularItemRowConsumption_ = NeatPacker::itemWidth_;
            yRotatedRow.nrOfItems_ = (int)std::floor(NeatPacker::binHeight_ / NeatPacker::itemWidth_);
            yRotatedRow.height_ = yRotatedRow.nrOfItems_ * NeatPacker::itemWidth_;

            xRotatedRow.valid_ = (NeatPacker::binWidth_ >= xRotatedRow.width_ && NeatPacker::binHeight_ >= xRotatedRow.singularItemRowConsumption_);
            yRotatedRow.valid_ = (NeatPacker::binWidth_ >= yRotatedRow.width_ && NeatPacker::binHeight_ >= yRotatedRow.singularItemRowConsumption_);
        }

        NeatPacker::rows_ = {xRotatedRow, yRotatedRow};

        // Determine the most desired type of row to place.
        mostValuableRow_ = (xRotatedRow.nrOfItems_ < yRotatedRow.nrOfItems_) ? yRotatedRow : xRotatedRow;
    }

    /// @brief Determines in which direction rows will be added. (not in which direction boxes will be added...).
    /// First, check if there is an axis that would fit more boxes, if there is, it wins.
    /// If the max is equal for both axi, check the combined value, if one is higher, it wins.
    /// If the combined value is equal, check which would have the least left over space, least wins.
    void determinePackingAxis()
    {
        int bestAxis = constants::axis::DEPTH;

        int xxItems = 0;
        int xyItems = 0;
        int yyItems = 0;
        int yxItems = 0;

        // Calculate how many items would fit on a row.
        if (NeatPacker::binHeight_ >= NeatPacker::itemHeight_ && NeatPacker::binWidth_ >= NeatPacker::itemWidth_)
        {
            yyItems = (int)std::floor(NeatPacker::binHeight_ / NeatPacker::itemHeight_);
        }
        if (NeatPacker::binWidth_ >= NeatPacker::itemWidth_ && NeatPacker::binHeight_ >= NeatPacker::itemHeight_)
        {
            xxItems = (int)std::floor(NeatPacker::binWidth_ / NeatPacker::itemWidth_);
        }

        if (NeatPacker::binWidth_ >= NeatPacker::itemHeight_ && NeatPacker::binHeight_ >= NeatPacker::itemWidth_)
        {
            xyItems = (int)std::floor(NeatPacker::binWidth_ / NeatPacker::itemHeight_);
        }

        if (NeatPacker::binHeight_ >= NeatPacker::itemWidth_ && NeatPacker::binWidth_ >= NeatPacker::itemHeight_)
        {
            yxItems = (int)std::floor(NeatPacker::binHeight_ / NeatPacker::itemWidth_);
        }

        if (std::max(xxItems, xyItems) < std::max(yxItems, yyItems))
        {
            bestAxis = constants::axis::WIDTH;
        }
        else if (std::max(xxItems, xyItems) == std::max(yxItems, yyItems))
        {
            const int xxleftOverSpace = NeatPacker::binWidth_ % NeatPacker::itemWidth_;
            const int xyleftOverSpace = NeatPacker::binWidth_ % NeatPacker::itemHeight_;
            const int yyleftOverSpace = NeatPacker::binHeight_ % NeatPacker::itemHeight_;
            const int yxleftOverSpace = NeatPacker::binHeight_ % NeatPacker::itemWidth_;
            if (std::min(xxleftOverSpace, xyleftOverSpace) > std::min(yyleftOverSpace, yxleftOverSpace))
            {
                bestAxis = constants::axis::WIDTH;
            }
        }

        NeatPacker::stackingAxis_ = bestAxis;
    }

    /// @brief Returns the next item that should be created.
    /// Eventhough the algorithm works on a row to row basis, the items are returned individually.
    /// This is necessary because you might hit a weight restriction.
    /// @return const std::array<int, 4>
    /// nextItemsToMake[0] - Boolean, indicates if item was packed or not.
    /// nextItemsToMake[1] - int, xCoordinate of the item.
    /// nextItemsToMake[2] - int, yCoordinate of the item.
    /// nextItemsToMake[3] - boolean, indicates if the item was rotated.
    const std::array<int, 4> getNextItemToMake()
    {
        int xLocation = 0;
        int yLocation = 0;
        bool isRotated = 0;
        bool generatedItem = false;

        // Check if there are pre-computed items from a previous recursion.
        if (NeatPacker::hasRecursed_ && !NeatPacker::innerPackedItems_.empty())
        {
            Rect existingPreComputedNode = NeatPacker::consumePreComputedItem();
            isRotated = existingPreComputedNode.width != NeatPacker::itemWidth_;
            generatedItem = true;
            return {generatedItem, existingPreComputedNode.x, existingPreComputedNode.y, isRotated};
        };

        // If no items are calculated to be placed, try to place a new row.
        if (!NeatPacker::packingAction_->getHasActionsLeft())
        {
            NeatPacker::placeNewRow();
        }

        // If still no new row can be placed, return.
        if (!NeatPacker::packingAction_->getHasActionsLeft())
        {
            return {generatedItem, xLocation, yLocation, isRotated};
        }

        const int itemNumberInRow = NeatPacker::packingAction_->getNrOfExcutedItemActions();
        const int itemRotationAxis = NeatPacker::packingAction_->getItemRotationAxis();
        const RectRow rowToBeUsed = NeatPacker::rows_[itemRotationAxis];

        // Start packing the items that have been indicated by the row that was placed.
        if (NeatPacker::stackingAxis_ == constants::axis::DEPTH)
        {
            xLocation = (itemNumberInRow == 0 ? 0 : NeatPacker::packingAction_->getNrOfExcutedItemActions() * rowToBeUsed.singularItemRowConsumption_);
            yLocation = NeatPacker::binHeight_ - (NeatPacker::getMyStackingDistance() + rowToBeUsed.height_);
        }
        else if (NeatPacker::stackingAxis_ == constants::axis::WIDTH)
        {
            xLocation = NeatPacker::binWidth_ - (NeatPacker::getMyStackingDistance() + rowToBeUsed.stackingSpaceConsumption_);
            yLocation = (itemNumberInRow == 0 ? 0 : NeatPacker::packingAction_->getNrOfExcutedItemActions() * rowToBeUsed.singularItemRowConsumption_);
        }

        isRotated = (NeatPacker::rows_[constants::axis::DEPTH].axis_ == itemRotationAxis);

        // Increment the number of items that we have executed for this packing action.
        NeatPacker::packingAction_->setNrOfExcutedItemActions(NeatPacker::packingAction_->getNrOfExcutedItemActions() + 1);

        generatedItem = true;

        return {generatedItem, xLocation, yLocation, isRotated};
    };

    /// @brief Determine which row should be placed next.
    /// This sets the nextItemsToMake, which will then get consumed by the packer.
    /// Once the row is fully placed, this function will be called again to set the next row.
    void placeNewRow()
    {
        if (NeatPacker::getMyStackingDistance() < std::min(NeatPacker::rows_[constants::axis::WIDTH].stackingSpaceConsumption_,
                                                           NeatPacker::rows_[constants::axis::DEPTH].stackingSpaceConsumption_))
        {
            NeatPacker::packingAction_->reset();
            return;
        }

        // Initialize the next row.
        RectRow nextRow = {};

        // Determine secondary choice row.
        RectRow secondaryRow = NeatPacker::getSecondaryRow();

        const int leftOverRowsP = (int)std::floor(NeatPacker::getMyStackingDistance() / NeatPacker::mostValuableRow_.stackingSpaceConsumption_);
        const int leftOverRowsS = (int)std::floor(NeatPacker::getMyStackingDistance() / secondaryRow.stackingSpaceConsumption_);

        // Only 1 primary row left but more than 1 secondary row left.
        if (leftOverRowsP <= 1 &&
            leftOverRowsS > 0 &&
            leftOverRowsP < leftOverRowsS &&
            secondaryRow.valid_)
        {
            // Secondary row is chosen.
            if (leftOverRowsP == 1 && canRecurse_)
            {
                NeatPacker::recurseInEmptySpace(secondaryRow);
            }

            NeatPacker::subtractMyStackingDistance(secondaryRow.stackingSpaceConsumption_);
            nextRow = secondaryRow;
        }
        else if (leftOverRowsP > 0)
        {
            // Primary row is chosen.
            NeatPacker::subtractMyStackingDistance(mostValuableRow_.stackingSpaceConsumption_);
            nextRow = mostValuableRow_;
        };

        NeatPacker::packingAction_->setNextAction(nextRow.axis_, nextRow.nrOfItems_);
    }

    /// @brief Evaluate if an extra item will fit on the secondary row.
    /// @param aRow
    void recurseInEmptySpace(const RectRow aRow)
    {
        int temporaryBinSizeX = 0;
        int temporaryBinSizeY = 0;
        int innerBinStartPositionX = 0;
        int innerBinStartPositionY = 0;

        if (NeatPacker::stackingAxis_ == constants::axis::DEPTH)
        {
            temporaryBinSizeX = NeatPacker::binWidth_ - aRow.width_;
            temporaryBinSizeY = NeatPacker::getMyStackingDistance();
            innerBinStartPositionX = aRow.width_;
            innerBinStartPositionY = binHeight_ - temporaryBinSizeY;
        }
        else if (NeatPacker::stackingAxis_ == constants::axis::WIDTH)
        {
            temporaryBinSizeX = NeatPacker::getMyStackingDistance();
            temporaryBinSizeY = NeatPacker::binHeight_ - aRow.height_;
            innerBinStartPositionX = binWidth_ - temporaryBinSizeX;
            innerBinStartPositionY = aRow.height_;
        }

        const bool xFits = (innerBinStartPositionX + NeatPacker::getMostValuableItemWidth()) <= binWidth_;
        const bool yFits = (innerBinStartPositionY + NeatPacker::getMostValuableItemHeight()) <= binHeight_;

        // Set canRecurse to false, this way our inner NeatPacker will not recurse.
        // Also, this way if xFits or yFits is not true, we wont be recursing again.
        NeatPacker::canRecurse_ = false;

        if (xFits && yFits)
        {
            // Store original values in variables so we can reconfigure the 'real' bin.
            const int originalBinWidth = binWidth_;
            const int originalBinHeight = binHeight_;
            const std::array<int, 2> originalStackingDistance = stackingDistance_;

            // Reconfigure the NeatPacker to try to pack in the empty space created by primary and secondary row combination.
            NeatPacker::reConfigure(temporaryBinSizeX, temporaryBinSizeY);

            // Variable to prevent endless looping incase something goes wrong.
            int innerNeatPackCounter = 0;
            while (innerNeatPackCounter < 5000)
            {
                Rect newNode = {};
                newNode = NeatPacker::insert();

                if (newNode.width == 0)
                {
                    break;
                }
                else
                {
                    newNode.x += innerBinStartPositionX;
                    newNode.y += innerBinStartPositionY;
                    NeatPacker::innerPackedItems_.push_back(newNode);
                    innerNeatPackCounter++;
                }
            }

            // Recursing is done, restore the original settings.
            NeatPacker::reConfigure(originalBinWidth, originalBinHeight);
            NeatPacker::stackingDistance_ = originalStackingDistance;
            NeatPacker::hasRecursed_ = true;
        }
    }

public:
    NeatPacker(const int aWidth,
               const int aHeight,
               const int aItemWidth,
               const int aItemHeight) : binWidth_(aWidth),
                                        binHeight_(aHeight),
                                        itemWidth_(aItemWidth),
                                        itemHeight_(aItemHeight),
                                        stackingDistance_({aWidth, aHeight}),
                                        hasRecursed_(false),
                                        canRecurse_(true)
    {

        NeatPacker::determinePackingAxis();
        NeatPacker::initializeRows();
        NeatPacker::packingAction_ = std::make_unique<PackingAction>();
    };

    /// @brief Reset the algorithm.
    void reset()
    {
        NeatPacker::determinePackingAxis();
        NeatPacker::initializeRows();
        NeatPacker::stackingDistance_ = {binWidth_, binHeight_};
        NeatPacker::packingAction_->reset();
    }

    /// @brief Reconfigure and reset the algorithm according to given bin size.
    /// @param aBinWidth
    /// @param aBinHeight
    void reConfigure(const int aBinWidth, const int aBinHeight)
    {
        NeatPacker::binWidth_ = aBinWidth;
        NeatPacker::binHeight_ = aBinHeight;
        NeatPacker::reset();
    }

    /// @brief Insert a new rectangle.
    /// @return Rect
    Rect insert()
    {
        Rect newRect = {};
        newRect.width = 0;
        newRect.height = 0;
        newRect.x = 0;
        newRect.y = 0;

        if (!(rows_[constants::axis::WIDTH].valid_ || rows_[constants::axis::DEPTH].valid_))
        {
            return newRect;
        }

        const std::array<int, 4> nextItemsToMake = NeatPacker::getNextItemToMake();

        // Item fits, set attributes accordingly.
        if (nextItemsToMake[0])
        {
            // Check if item was rotated, if so swap dimensions.
            newRect.width = (nextItemsToMake[3] ? itemHeight_ : itemWidth_);
            newRect.height = (nextItemsToMake[3] ? itemWidth_ : itemHeight_);
            newRect.x = nextItemsToMake[1];
            newRect.y = nextItemsToMake[2];
        }

        return newRect;
    }
};

#endif