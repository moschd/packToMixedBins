#ifndef BIN_H
#define BIN_H

class Bin : public GeometricShape, public BinCalculationCache
{
private:
    std::vector<int> items_;
    std::vector<int> unfittedItems_;
    std::vector<int> xFreeItems_;
    std::vector<int> yFreeItems_;
    std::vector<int> zFreeItems_;
    std::array<double, 3> placedItemsMaxDimensions_;
    std::array<double, 3> placedItemsFurthestPoints_;
    double actualVolumeUtil_;
    double actualWeightUtil_;
    PackingContext *context_;

    /**
     * @brief Removes items from the xfree axis when an item has been placed there.
     *
     * @param itemBeingPlaced
     */
    void removeFromXFreeItems(const Item *itemBeingPlaced)
    {
        Bin::xFreeItems_.erase(
            std::remove_if(begin(Bin::xFreeItems_), end(Bin::xFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                    const Item* itemInBin = &Bin::context_->getItem(itemInBinKey);
                    return (
                            itemBeingPlaced->Item::position_[constants::axis::WIDTH] == itemInBin->Item::furthestPointWidth_ &&
                            Geometry::intersectingY(itemBeingPlaced,itemInBin) && Geometry::intersectingZ(itemBeingPlaced,itemInBin)); }),
            end(xFreeItems_));
    };

    /**
     * @brief Removes items from the yfree axis when an item has been placed there.
     *
     * @param itemBeingPlaced
     */
    void removeFromYFreeItems(const Item *itemBeingPlaced)
    {
        Bin::yFreeItems_.erase(
            std::remove_if(begin(Bin::yFreeItems_), end(Bin::yFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                const Item* itemInBin = &context_->getItem(itemInBinKey);
                return (
                        itemBeingPlaced->Item::position_[constants::axis::DEPTH] == itemInBin->Item::furthestPointDepth_ && 
                        Geometry::intersectingX(itemBeingPlaced,itemInBin) && Geometry::intersectingZ(itemBeingPlaced,itemInBin)); }),
            end(Bin::yFreeItems_));
    };

    /**
     * @brief Removes items from the zfree axis when an item has been placed there.
     *
     * @param itemBeingPlaced
     */
    void removeFromZFreeItems(const Item *itemBeingPlaced)
    {
        Bin::zFreeItems_.erase(
            std::remove_if(begin(Bin::zFreeItems_), end(Bin::zFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                const Item* itemInBin = &context_->getItem(itemInBinKey);
                return (
                        itemBeingPlaced->Item::position_[constants::axis::HEIGHT] == itemInBin->Item::furthestPointHeight_ &&
                        Geometry::intersectingX(itemBeingPlaced,itemInBin) && Geometry::intersectingY(itemBeingPlaced,itemInBin)); }),
            end(Bin::zFreeItems_));
    };

    /**
     * @brief Keep track of the maximum dimensions that have been placed inside the bin.
     *
     * This is used to help narrow down the number of branches to search in the kd-tree.
     * When we know the maximum length of the item on a certain axis, we also know which
     * branches we can prune since no item would reach that far starting from a certain point.
     *
     * @param it
     * @param axis
     */
    void updatePlacedMaxItemDimensions(const Item *it, const int axis)
    {
        switch (axis)
        {
        case constants::axis::WIDTH:
            Bin::placedItemsMaxDimensions_[constants::axis::WIDTH] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::WIDTH], it->Item::width_);
            break;
        case constants::axis::DEPTH:
            Bin::placedItemsMaxDimensions_[constants::axis::DEPTH] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::DEPTH], it->Item::depth_);
            break;
        case constants::axis::HEIGHT:
            Bin::placedItemsMaxDimensions_[constants::axis::HEIGHT] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::HEIGHT], it->Item::height_);
            break;
        };
    };

    /**
     * @brief Get the furthest position of any item on the pallet.
     *
     * Can be used to calculate product underhang.
     *
     * @param it
     * @param axis
     */
    void updatePlacedMaxItemPositions(const Item *it, const int axis)
    {
        Bin::placedItemsFurthestPoints_[constants::axis::WIDTH] = std::max(Bin::placedItemsFurthestPoints_[constants::axis::WIDTH], it->Item::furthestPointWidth_);
        Bin::placedItemsFurthestPoints_[constants::axis::DEPTH] = std::max(Bin::placedItemsFurthestPoints_[constants::axis::DEPTH], it->Item::furthestPointDepth_);
        Bin::placedItemsFurthestPoints_[constants::axis::HEIGHT] = std::max(Bin::placedItemsFurthestPoints_[constants::axis::HEIGHT], it->Item::furthestPointHeight_);
    };

    /**
     * @brief Update everything that needs to be updated once an item has found a fitting spot inside the bin.
     *
     * @param it
     * @param binAxis
     */
    void updateWithFittedItem(const int &it, const int binAxis)
    {
        Bin::items_.push_back(it);
        const Item *itemOb = &context_->getItem(it);
        Bin::actualWeightUtil_ += itemOb->Item::weight_;
        Bin::actualVolumeUtil_ += itemOb->Item::volume_;

        Bin::updatePlacedMaxItemDimensions(itemOb, binAxis);
        Bin::updatePlacedMaxItemPositions(itemOb, binAxis);
        Bin::kdTree_->KdTree::addItemKeyToLeafHelper(it,
                                                     {itemOb->Item::furthestPointWidth_,
                                                      itemOb->Item::furthestPointDepth_,
                                                      itemOb->Item::furthestPointHeight_});

        /* Insert the new item based on sorted height, this is to evaluate lowest height first when stacking upwards.*/
        const auto hiter = std::upper_bound(Bin::zFreeItems_.cbegin(), Bin::zFreeItems_.cend(), it, [&](const int i1, const int i2)
                                            { return context_->getItem(i1).Item::furthestPointHeight_ < context_->getItem(i2).Item::furthestPointHeight_; });
        Bin::xFreeItems_.push_back(it);
        Bin::yFreeItems_.push_back(it);
        Bin::zFreeItems_.insert(hiter, it);
        Bin::removeFromXFreeItems(itemOb);
        Bin::removeFromZFreeItems(itemOb);
        Bin::removeFromYFreeItems(itemOb);
    };

public:
    int id_;
    std::string type_;
    double maxWeight_;
    KdTree *kdTree_;

    Bin(int aId,
        PackingContext *aContext,
        int aEstimatedNumberOfItemFits) : id_(aId),
                                          type_(aContext->getRequestedBin()->getType()),
                                          maxWeight_(aContext->getRequestedBin()->getMaxWeight()),
                                          actualVolumeUtil_(0.0),
                                          actualWeightUtil_(0.0),
                                          context_(aContext),
                                          placedItemsMaxDimensions_(constants::START_POSITION),
                                          placedItemsFurthestPoints_(constants::START_POSITION),
                                          GeometricShape(aContext->getRequestedBin()->getWidth(),
                                                         aContext->getRequestedBin()->getDepth(),
                                                         aContext->getRequestedBin()->getHeight())
    {

        /* Create kd-tree and reserve vector memory in advance based on estimates. */
        Bin::kdTree_ = new KdTree(aEstimatedNumberOfItemFits, {width_, depth_, height_});
        Bin::items_.reserve(aEstimatedNumberOfItemFits);
        Bin::xFreeItems_.reserve(aEstimatedNumberOfItemFits);
        Bin::yFreeItems_.reserve(aEstimatedNumberOfItemFits);
        Bin::zFreeItems_.reserve(aEstimatedNumberOfItemFits);
    };

    const std::vector<int> &getFittedItems() const
    {
        return Bin::items_;
    };

    const std::vector<int> &getUnfittedItems() const
    {
        return Bin::unfittedItems_;
    };

    const std::array<double, 3> getPlacedItemsFurthestPoints() const
    {
        return Bin::placedItemsFurthestPoints_;
    }

    /**
     * @brief Reset the item to inital values and add to unfitted items.
     *
     * @param itemKey
     */
    void addUnfittedItem(const int itemKey)
    {
        Bin::context_->getModifiableItem(itemKey).reset();
        Bin::unfittedItems_.push_back(itemKey);
    };

    const double getActVolumeUtilPercentage() const
    {
        return Bin::actualVolumeUtil_ / GeometricShape::volume_ * 100;
    };

    const double getActWeightUtilPercentage() const
    {
        return Bin::actualWeightUtil_ / Bin::maxWeight_ * 100;
    };

    const double getActWeightUtil() const
    {
        return Bin::actualWeightUtil_;
    };

    const double getActVolumeUtil() const
    {
        return Bin::actualVolumeUtil_;
    };

    /**
     * @brief Helper function to be able to update the bin without exposing the complete function.
     *
     * @param it
     * @param binAxis
     */
    void updateWithFittedItemHelper(const int &it, const int binAxis)
    {
        Bin::updateWithFittedItem(it, binAxis);
    };

    void generateItemsOnXFromItem(const int aBaseItemKey)
    {
        const Item *baseItem = &Bin::context_->getItem(aBaseItemKey);
        const double itemWeight = baseItem->weight_;

        double incrementalWidth = baseItem->furthestPointWidth_;

        //    (Bin::getActWeightUtil() + itemWeight) < context_->getRequestedBin()->getMaxWeight()
        while (Bin::width_ > (incrementalWidth + baseItem->width_))
        {

            const Item *previousItem = &Bin::context_->getItem(context_->getNumberOfCreatedItems());
            const int newItemId = context_->getNumberOfCreatedItems() + 1;
            Item newItem(newItemId,
                         "newItem",
                         baseItem->width_,
                         baseItem->depth_,
                         baseItem->height_,
                         baseItem->weight_,
                         baseItem->itemConsolidationKey_,
                         baseItem->allowedRotations_,
                         baseItem->gravityStrength_);

            Bin::context_->addItemToRegister(newItem);

            Item *retrievedNewItem = &Bin::context_->getModifiableItem(newItemId);

            retrievedNewItem->Item::position_ = previousItem->position_;
            retrievedNewItem->Item::position_[constants::axis::WIDTH] += baseItem->Item::width_;

            incrementalWidth += baseItem->width_;

            Bin::updateWithFittedItem(retrievedNewItem->transientSysId_, constants::axis::WIDTH);

            std::cout << "Created item=" << retrievedNewItem->transientSysId_ << " "
                      << "X=" << retrievedNewItem->position_[constants::axis::WIDTH] << "\n";
        }
    }

    void generateItemsOnYFromItem(const int aBaseItemKey)
    {
        const Item *baseItem = &Bin::context_->getItem(aBaseItemKey);

        double incrementalDepth = baseItem->position_[constants::axis::DEPTH] + baseItem->depth_;

        //    (Bin::getActWeightUtil() + itemWeight) < context_->getRequestedBin()->getMaxWeight()
        while (Bin::depth_ > (incrementalDepth + baseItem->depth_))
        {

            const Item *previousItem = &Bin::context_->getItem(context_->getNumberOfCreatedItems());
            const int newItemId = context_->getNumberOfCreatedItems() + 1;
            Item newItem(newItemId,
                         "newItem",
                         baseItem->width_,
                         baseItem->depth_,
                         baseItem->height_,
                         baseItem->weight_,
                         baseItem->itemConsolidationKey_,
                         baseItem->allowedRotations_,
                         baseItem->gravityStrength_);

            Bin::context_->addItemToRegister(newItem);

            Item *retrievedNewItem = &Bin::context_->getModifiableItem(newItemId);

            retrievedNewItem->Item::position_ = previousItem->position_;
            retrievedNewItem->Item::position_[constants::axis::DEPTH] += baseItem->Item::depth_;

            incrementalDepth += baseItem->depth_;

            Bin::updateWithFittedItem(retrievedNewItem->transientSysId_, constants::axis::DEPTH);

            std::cout << "Created item=" << retrievedNewItem->transientSysId_ << " "
                      << "Y=" << retrievedNewItem->position_[constants::axis::DEPTH] << "\n";
        }
    }

    void constructMasterLayer(const int aMasterItem)
    {
        Bin::updateWithFittedItem(aMasterItem, constants::axis::WIDTH);
        Bin::constructLayer(aMasterItem);
    }
    /**
     * @brief
     *
     *
     *
    take item width, add it until item would exceed bin.
        if it would exceed.
        take current distance - max distance, turn in % relative to max

    take item depth, add it until item would exceed bin.
        if it would exceed.
        take current distance - max distance, turn in % relative to max

    rotate

    take item width, add it until item would exceed bin.
        if it would exceed.
        take current distance - max distance, turn in % relative to max

    take item depth, add it until item would exceed bin.
        if it would exceed.
        take current distance - max distance, turn in % relative to max
     *
     * @param itemToFitKey
     */
    void constructLayer(const int &itemToFitKey)
    {

        Item *baseItem = &Bin::context_->getModifiableItem(itemToFitKey);

        int numberOfItemsWidth = 1;
        const double startPositionWidth = baseItem->position_[constants::axis::WIDTH];
        const double startPositionDepth = baseItem->position_[constants::axis::DEPTH];

        std::cout << "STARTING FROM WIDTH=" << startPositionWidth << " "
                  << "STARTING FROM DEPTH=" << startPositionDepth << "\n";
        double incrementalWidth = baseItem->width_;
        while (Bin::width_ > (startPositionWidth + incrementalWidth + baseItem->width_))
        {
            incrementalWidth += baseItem->width_;
            numberOfItemsWidth++;
        }

        const bool xFit = numberOfItemsWidth > 1;
        double xFreeDistance = Bin::width_ - incrementalWidth;
        std::cout << "WIDTH " << baseItem->width_ << " " << numberOfItemsWidth << " " << xFreeDistance << "\n";

        int numberOfItemsDepth = 1;
        double incrementalDepth = baseItem->depth_;
        while (Bin::depth_ > (startPositionDepth + incrementalDepth + baseItem->depth_))
        {
            incrementalDepth += baseItem->depth_;
            numberOfItemsDepth++;
        }
        const bool yFit = numberOfItemsDepth > 1;
        double yFreeDistance = Bin::depth_ - incrementalDepth;
        std::cout << "DEPTH " << baseItem->depth_ << " " << numberOfItemsDepth << " " << yFreeDistance << "\n";

        baseItem->rotate(constants::rotation::type::DWH);

        std::cout << "ROTATING ITEM\n";

        int numberOfItemsWidth2 = 1;
        double incrementalWidth2 = baseItem->width_;
        while (Bin::width_ > (startPositionWidth + incrementalWidth2 + baseItem->width_))
        {
            incrementalWidth2 += baseItem->width_;
            numberOfItemsWidth2++;
        }
        const bool xFit2 = numberOfItemsWidth2 > 1;
        double xFreeDistance2 = Bin::width_ - incrementalWidth2;
        std::cout << "WIDTH2 " << baseItem->width_ << " " << numberOfItemsWidth2 << " " << xFreeDistance2 << "\n";

        int numberOfItemsDepth2 = 1;
        double incrementalDepth2 = baseItem->depth_;
        while (Bin::depth_ > (startPositionDepth + incrementalDepth2 + baseItem->depth_))
        {
            incrementalDepth2 += baseItem->depth_;
            numberOfItemsDepth2++;
        }
        const bool yFit2 = numberOfItemsDepth2 > 1;
        double yFreeDistance2 = Bin::depth_ - incrementalDepth2;
        std::cout << "DEPTH2 " << baseItem->depth_ << " " << numberOfItemsDepth2 << " " << yFreeDistance2 << "\n";

        baseItem->reset();

        const double mostEfficientDirection = std::min(std::min(xFreeDistance, yFreeDistance), std::min(xFreeDistance2, xFreeDistance2));
        std::cout << mostEfficientDirection << "\n";
        if (xFit && xFreeDistance == mostEfficientDirection)
        {
            std::cout << "Going here1\n";
            generateItemsOnXFromItem(baseItem->transientSysId_);
        }
        else if (yFit && yFreeDistance == mostEfficientDirection)
        {
            std::cout << "Going here2 " << baseItem->transientSysId_ << "\n";
            generateItemsOnYFromItem(baseItem->transientSysId_);

            const int newItemId = context_->getNumberOfCreatedItems() + 1;
            Item newItem(newItemId,
                         "newItem",
                         baseItem->width_,
                         baseItem->depth_,
                         baseItem->height_,
                         baseItem->weight_,
                         baseItem->itemConsolidationKey_,
                         baseItem->allowedRotations_,
                         baseItem->gravityStrength_);

            Bin::context_->addItemToRegister(newItem);
            Item *retrievedNewItem = &Bin::context_->getModifiableItem(newItemId);

            retrievedNewItem->Item::position_ = baseItem->position_;
            retrievedNewItem->Item::position_[constants::axis::WIDTH] += baseItem->Item::width_;

            std::cout << "NR_OF_ITEMS=" << context_->getNumberOfCreatedItems() << "\n";
            constructLayer(newItemId);
        }
        else if (xFit2 && xFreeDistance2 == mostEfficientDirection)
        {
            std::cout << "Going here3\n";
            baseItem->rotate(constants::rotation::type::DWH);
            generateItemsOnXFromItem(baseItem->transientSysId_);
        }
        else if (yFit2 && yFreeDistance2 == mostEfficientDirection)
        {
            std::cout << "Going here4\n";
            baseItem->rotate(constants::rotation::type::DWH);
            generateItemsOnYFromItem(baseItem->transientSysId_);
        };

        std::cout << "Numer of items=" << context_->getNumberOfCreatedItems() << "\n";
    }

    /**
     * @brief Look for a position inside the bin to place the item.
     *
     * @param itemToFitKey
     */
    void findItemPosition(const int &itemToFitKey)
    {
        bool fitted = false;

        std::vector<int> itemsWithFreeCorrespondingAxis;
        Item *itemToFit = &Bin::context_->getModifiableItem(itemToFitKey);

        for (const auto binAxis : Bin::context_->getPackingDirection())
        {
            switch (binAxis)
            {
            case constants::axis::WIDTH:
                itemsWithFreeCorrespondingAxis = Bin::xFreeItems_;
                break;
            case constants::axis::DEPTH:
                itemsWithFreeCorrespondingAxis = Bin::yFreeItems_;
                break;
            case constants::axis::HEIGHT:
                itemsWithFreeCorrespondingAxis = Bin::zFreeItems_;
                break;
            };

            for (const auto itemInBinKey : itemsWithFreeCorrespondingAxis)
            {
                const Item *itemInBin = &Bin::context_->getItem(itemInBinKey);
                itemToFit->Item::position_ = itemInBin->Item::position_;

                switch (binAxis)
                {
                case constants::axis::WIDTH:
                    itemToFit->Item::position_[constants::axis::WIDTH] += itemInBin->Item::width_;
                    break;
                case constants::axis::DEPTH:
                    itemToFit->Item::position_[constants::axis::DEPTH] += itemInBin->Item::depth_;
                    break;
                case constants::axis::HEIGHT:
                    itemToFit->Item::position_[constants::axis::HEIGHT] += itemInBin->Item::height_;
                    break;
                };

                if (BinCalculationCache::itemPositionCacheHit(itemToFit))
                {
                    continue;
                };

                if (Bin::placeItemInBin(itemToFitKey))
                {
                    fitted = true;
                }

                if (fitted)
                {
                    Bin::updateWithFittedItem(itemToFitKey, binAxis);
                    break;
                }
            };
            if (fitted)
            {
                break;
            };
        };
        if (!fitted)
        {
            Bin::addUnfittedItem(itemToFitKey);
        };
    };

    /**
     * @brief Tries to place an item inside a bin.
     *
     * @param it
     * @return true
     * @return false
     */
    const bool placeItemInBin(const unsigned int aItemBeingPlacedKey)
    {
        bool intersectionFound = false;
        Item *itemBeingPlaced = &Bin::context_->getModifiableItem(aItemBeingPlacedKey);

        /* Loop over items allowed rotation in order to find a fitting place. */
        for (int stringCharCounter = 0;
             stringCharCounter < itemBeingPlaced->Item::allowedRotations_.std::string::size();
             stringCharCounter++)
        {
            /* Rotate item according to current rotation type. */
            itemBeingPlaced->Item::rotate(itemBeingPlaced->Item::allowedRotations_[stringCharCounter] - '0');

            /* Check if item is not exceeding the bin dimensions, if so try a different rotation. */
            if (Bin::width_ < itemBeingPlaced->Item::furthestPointWidth_ ||
                Bin::depth_ < itemBeingPlaced->Item::furthestPointDepth_ ||
                Bin::height_ < itemBeingPlaced->Item::furthestPointHeight_)
            {
                continue;
            };

            /* Search kdTree to find items which could be intersecting. */
            std::vector<int> intersectCandidates;
            Bin::kdTree_->getIntersectCandidates(Bin::kdTree_->KdTree::getRoot(),
                                                 Bin::kdTree_->KdTree::getRoot()->Node::myDepth_,
                                                 itemBeingPlaced->Item::position_,
                                                 {Bin::placedItemsMaxDimensions_[constants::axis::WIDTH] + itemBeingPlaced->Item::width_,
                                                  Bin::placedItemsMaxDimensions_[constants::axis::DEPTH] + itemBeingPlaced->Item::depth_,
                                                  Bin::placedItemsMaxDimensions_[constants::axis::HEIGHT] + itemBeingPlaced->Item::height_},
                                                 intersectCandidates);

            /* Iterate over candidates and check for collision. */
            for (auto intersectCandidateKey : intersectCandidates)
            {
                const Item *intersectCandidate = &Bin::context_->getItem(intersectCandidateKey);

                /*  Check if X and Y axis are intersecting with a item already placed in the bin. */
                if (!Geometry::intersectingXY(itemBeingPlaced, intersectCandidate))
                {
                    continue;
                };

                /*  X and Y axis were found to be intersecting, check Z axis.
                    If also intersects then create cache entry and break out of current rotation type loop. */
                if (Geometry::intersectingZ(itemBeingPlaced, intersectCandidate))
                {
                    BinCalculationCache::addIntersection(itemBeingPlaced, intersectCandidate);
                    intersectionFound = true;
                    break;
                };
            };

            /* Current rotation type was found to be intersecting with an already placed item. Try new rotation type. */
            if (intersectionFound)
            {
                intersectionFound = false;
                continue;
            };

            /*  Checks if gravity should be considered while placing this item.
            This check is applied when an otherwise fitting item is found. */
            if (!Bin::context_->itemObeysGravity(itemBeingPlaced, Bin::getFittedItems()))
            {
                continue;
            };

            /* If this point is reached then the item fits in the bin. */
            return true;
        };

        /* If this point is reached then the item didnt find a place. */
        return false;
    };
};

#endif