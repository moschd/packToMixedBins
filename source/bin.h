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
    double actualVolumeUtil_;
    double actualWeightUtil_;
    Gravity *masterGravity_;
    ItemRegister *masterItemRegister_;

    void removeFromXFreeItems(const Item *itemBeingPlaced)
    {
        Bin::xFreeItems_.erase(
            std::remove_if(begin(Bin::xFreeItems_), end(Bin::xFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                    const Item* itemInBin = &Bin::masterItemRegister_->ItemRegister::getConstItem(itemInBinKey);
                    return (
                            itemBeingPlaced->Item::position_[constants::axis::WIDTH] == itemInBin->Item::furthestPointWidth_ &&
                            Geometry::intersectingY(itemBeingPlaced,itemInBin) && Geometry::intersectingZ(itemBeingPlaced,itemInBin)); }),
            end(xFreeItems_));
    };

    void removeFromYFreeItems(const Item *itemBeingPlaced)
    {
        Bin::yFreeItems_.erase(
            std::remove_if(begin(Bin::yFreeItems_), end(Bin::yFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                const Item* itemInBin = &masterItemRegister_->ItemRegister::getConstItem(itemInBinKey);
                return (
                        itemBeingPlaced->Item::position_[constants::axis::DEPTH] == itemInBin->Item::furthestPointDepth_ && 
                        Geometry::intersectingX(itemBeingPlaced,itemInBin) && Geometry::intersectingZ(itemBeingPlaced,itemInBin)); }),
            end(Bin::yFreeItems_));
    };

    void removeFromZFreeItems(const Item *itemBeingPlaced)
    {
        Bin::zFreeItems_.erase(
            std::remove_if(begin(Bin::zFreeItems_), end(Bin::zFreeItems_), [&](int &itemInBinKey) -> bool
                           { 
                const Item* itemInBin = &masterItemRegister_->ItemRegister::getConstItem(itemInBinKey);
                return (
                        itemBeingPlaced->Item::position_[constants::axis::HEIGHT] == itemInBin->Item::furthestPointHeight_ &&
                        Geometry::intersectingX(itemBeingPlaced,itemInBin) && Geometry::intersectingY(itemBeingPlaced,itemInBin)); }),
            end(Bin::zFreeItems_));
    };

    /**
     * @brief Keep track of the maximum dimensions that have been placed inside the bin.
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
        default:
            Bin::placedItemsMaxDimensions_[constants::axis::WIDTH] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::WIDTH], it->Item::width_);
            Bin::placedItemsMaxDimensions_[constants::axis::DEPTH] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::DEPTH], it->Item::depth_);
            Bin::placedItemsMaxDimensions_[constants::axis::HEIGHT] = std::max(Bin::placedItemsMaxDimensions_[constants::axis::HEIGHT], it->Item::height_);
            break;
        };
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
        const Item *itemOb = &masterItemRegister_->ItemRegister::getConstItem(it);
        Bin::actualWeightUtil_ += itemOb->Item::weight_;
        Bin::actualVolumeUtil_ += itemOb->Item::volume_;

        Bin::updatePlacedMaxItemDimensions(itemOb, binAxis);
        Bin::kdTree_->KdTree::addItemKeyToLeafHelper(it,
                                                     {itemOb->Item::furthestPointWidth_,
                                                      itemOb->Item::furthestPointDepth_,
                                                      itemOb->Item::furthestPointHeight_});

        // Insert the new item based on sorted height, this is to evaluate lowest height first when stacking upwards.
        const auto hiter = std::upper_bound(Bin::zFreeItems_.cbegin(), Bin::zFreeItems_.cend(), it, [&](const int i1, const int i2)
                                            { return masterItemRegister_->ItemRegister::getConstItem(i1).Item::furthestPointHeight_ < masterItemRegister_->ItemRegister::getConstItem(i2).Item::furthestPointHeight_; });
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
        RequestedBin *aRequestedBin,
        Gravity *aGravity,
        ItemRegister *aItemRegister,
        int aEstimatedNumberOfItemFits) : id_(aId),
                                          type_(aRequestedBin->getType()),
                                          maxWeight_(aRequestedBin->getMaxWeight()),
                                          actualVolumeUtil_(0.0),
                                          actualWeightUtil_(0.0),
                                          masterGravity_(aGravity),
                                          masterItemRegister_(aItemRegister),
                                          placedItemsMaxDimensions_(constants::START_POSITION),
                                          GeometricShape(aRequestedBin->getWidth(), aRequestedBin->getDepth(), aRequestedBin->getHeight())
    {

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

    /**
     * @brief Reset the item to inital values and add to unfitted items.
     *
     * @param itemKey
     */
    void addUnfittedItem(const int itemKey)
    {
        Item *itemBeingPlaced = &Bin::masterItemRegister_->ItemRegister::getItem(itemKey);
        itemBeingPlaced->reset();
        Bin::unfittedItems_.push_back(itemKey);
    };

    const double getActVolumeUtilizationPercentage() const
    {
        return Bin::actualVolumeUtil_ / GeometricShape::volume_ * 100;
    };

    const double getActWeightUtilizationPercentage() const
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

    /**
     * @brief Look for a position inside the bin to place the item.
     *
     * @param itemToFitKey
     */
    void findItemPosition(const int &itemToFitKey)
    {
        bool fitted = false;

        std::vector<int> itemsWithFreeCorrespondingAxis;
        Item *itemToFit = &masterItemRegister_->ItemRegister::getItem(itemToFitKey);

        for (const auto binAxis : constants::axis::ALL)
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
                const Item *itemInBin = &masterItemRegister_->ItemRegister::getConstItem(itemInBinKey);
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
     * This function can be considered the heart of the algorithm.
     *
     * @param it
     * @return true
     * @return false
     */
    const bool placeItemInBin(const unsigned int aItemBeingPlacedKey)
    {
        bool intersectionFound = false;
        Item *itemBeingPlaced = &Bin::masterItemRegister_->ItemRegister::getItem(aItemBeingPlacedKey);

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
                Item *intersectCandidate = &masterItemRegister_->ItemRegister::getItem(intersectCandidateKey);

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
            if (Bin::masterGravity_->Gravity::gravityEnabled(itemBeingPlaced))
            {
                if (!Bin::masterGravity_->Gravity::obeysGravity(itemBeingPlaced,
                                                                Bin::getFittedItems(),
                                                                Bin::masterItemRegister_))
                {
                    continue;
                };
            };

            /* If this point is reached then the item fits in the bin. */
            return true;
        };

        /* If this point is reached then the item didnt find a place. */
        return false;
    };
};

#endif