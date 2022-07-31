#ifndef BIN_H
#define BIN_H

#include "constants.h"
#include "miscfunctions.h"
#include "3r-kd-tree.h"

class Bin
{
private:
    double width_;
    double depth_;
    double height_;
    std::vector<int> items_;
    std::vector<int> unfittedItems_;
    double actualVolumeUtil_;
    double actualWeightUtil_;
    std::array<double, 3> topRightCorner_;

public:
    int id_;
    std::string type_;
    double maxWeight_;
    double maxVolume_;
    std::array<double, 3> placedItemsMaxDimensions_;

    std::vector<int> xFreeItems_;
    std::vector<int> yFreeItems_;
    std::vector<int> zFreeItems_;

    std::unordered_map<std::array<double, 3>, std::array<double, 3>, itemPositionHashFunction, itemPositionEqualsFunction> intersectPosDimCaching_;
    ItemRegister *passedOnMasterItemRegister_;
    Gravity *passedOnMasterGravity_;
    KdTree *kdTree2_;

    Bin(std::string aType, int aId, double aWidth, double aDepth, double aHeight, double aMaxWeight, Gravity *aGravity, ItemRegister *aItemRegister, int aEstimatedNumberOfItemFits)
    {
        id_ = aId;
        type_ = aType;
        width_ = aWidth;
        depth_ = aDepth;
        height_ = aHeight;
        maxVolume_ = width_ * depth_ * height_;
        topRightCorner_ = {width_, depth_, height_};

        maxWeight_ = aMaxWeight;
        passedOnMasterGravity_ = aGravity;
        passedOnMasterItemRegister_ = aItemRegister;
        placedItemsMaxDimensions_ = {0, 0, 0};
        actualVolumeUtil_ = 0.0;
        actualWeightUtil_ = 0.0;

        kdTree2_ = new KdTree(aEstimatedNumberOfItemFits, topRightCorner_);
    };

    std::vector<int> &GetFittedItems()
    {
        return items_;
    };
    std::vector<int> &GetUnfittedItems()
    {
        return unfittedItems_;
    };
    double getWidth()
    {
        return width_;
    };
    double getDepth()
    {
        return depth_;
    };
    double getHeight()
    {
        return height_;
    };
    double GetActVolumeUtilizationPercentage()
    {
        return actualVolumeUtil_ / maxVolume_ * 100;
    };
    double GetActWeightUtilizationPercentage()
    {
        return actualWeightUtil_ / maxWeight_ * 100;
    };
    double GetActWeightUtil()
    {
        return actualWeightUtil_;
    };
    double GetActVolumeUtil()
    {
        return actualVolumeUtil_;
    };

    void updatePlacedMaxItemDimensions(Item *it, int axis)
    {
        switch (axis)
        {
        case constants::AxisWidth:
            placedItemsMaxDimensions_[0] = std::max(placedItemsMaxDimensions_[0], it->width_);
            break;
        case constants::AxisDepth:
            placedItemsMaxDimensions_[1] = std::max(placedItemsMaxDimensions_[1], it->depth_);
            break;
        case constants::AxisHeight:
            placedItemsMaxDimensions_[2] = std::max(placedItemsMaxDimensions_[2], it->height_);
            break;
        };
    };
    void removeFromXFreeItems(const Item *it)
    {
        xFreeItems_.erase(
            std::remove_if(begin(xFreeItems_), end(xFreeItems_), [&](int &itemInBin) -> bool
                           { 
                    Item* iib = &passedOnMasterItemRegister_->getItem(itemInBin);
                    return (it->position_[0] == iib->ipwf_    &&
                            !(it->position_[1] >= iib->ipdf_  && 
                            it->ipdf_ <= iib->position_[1])   &&
                            !(it->position_[2] >= iib->iphf_  && 
                            it->iphf_ <= iib->position_[2])); }),
            end(xFreeItems_));
    };
    void removeFromYFreeItems(const Item *it)
    {
        yFreeItems_.erase(
            std::remove_if(begin(yFreeItems_), end(yFreeItems_), [&](int &itemInBin) -> bool
                           { 
                Item* iib = &passedOnMasterItemRegister_->getItem(itemInBin);
                return (it->position_[1] == iib->ipdf_    && 
                        !(it->position_[0] >= iib->ipwf_  && 
                        it->ipwf_ <= iib->position_[0])   &&
                        !(it->position_[2] >= iib->iphf_  && 
                        it->iphf_ <= iib->position_[2])); }),
            end(yFreeItems_));
    };
    void removeFromZFreeItems(const Item *it)
    {
        zFreeItems_.erase(
            std::remove_if(begin(zFreeItems_), end(zFreeItems_), [&](int &itemInBin) -> bool
                           { 
                Item* iib = &passedOnMasterItemRegister_->getItem(itemInBin);
                return (iib->iphf_ == it->position_[2]        &&
                        iib->position_[0] <= it->position_[0] && 
                        it->position_[0] <= iib->ipwf_        &&
                        iib->position_[1] <= it->position_[1] &&
                        it->position_[1] <= iib->ipdf_); }),
            end(zFreeItems_));
    };

    void UpdateWithNewFittedItem(int &it, int axis)
    {
        Item *itemOb = &passedOnMasterItemRegister_->getItem(it);

        // Add item to the bin
        items_.push_back(it);

        kdTree2_->addItemKeyToLeafHelper(it, {itemOb->ipwf_, itemOb->ipdf_, itemOb->iphf_});

        // Update the max placed item dimensions
        updatePlacedMaxItemDimensions(itemOb, axis);

        // Increment the bin weight/volume

        actualWeightUtil_ += itemOb->weight_;
        actualVolumeUtil_ += itemOb->volume_;

        // Add item to free item lists
        xFreeItems_.push_back(it);
        yFreeItems_.push_back(it);

        // Insert the new item based on sorted height
        auto hiter = std::upper_bound(zFreeItems_.cbegin(), zFreeItems_.cend(), it, [&](const int i1, const int i2)
                                      { return passedOnMasterItemRegister_->getItem(i1).iphf_ < passedOnMasterItemRegister_->getItem(i2).iphf_; });
        zFreeItems_.insert(hiter, it);

        // Remove items from potential items where positions are overlapping
        removeFromZFreeItems(itemOb);
        if (axis != constants::AxisHeight)
        {
            removeFromXFreeItems(itemOb);
            removeFromYFreeItems(itemOb);
        };
    };

    void FindItemPosition(int &itemToFitKey)
    {
        bool fitted = 0;

        std::vector<int> itemsWithFreeCorrespondingAxis;
        Item *itemToFit = &passedOnMasterItemRegister_->getItem(itemToFitKey);

        for (const auto axis : constants::AllAxis)
        {
            switch (axis)
            {
            case constants::AxisWidth:
                itemsWithFreeCorrespondingAxis = xFreeItems_;
                break;
            case constants::AxisDepth:
                itemsWithFreeCorrespondingAxis = yFreeItems_;
                break;
            case constants::AxisHeight:
                itemsWithFreeCorrespondingAxis = zFreeItems_;
                break;
            };

            for (const auto itemInBinKey : itemsWithFreeCorrespondingAxis)
            {
                Item *iib = &passedOnMasterItemRegister_->getItem(itemInBinKey);
                itemToFit->position_ = iib->position_;

                switch (axis)
                {
                case constants::AxisWidth:
                    itemToFit->position_[0] += iib->width_;
                    break;
                case constants::AxisDepth:
                    itemToFit->position_[1] += iib->depth_;
                    break;
                case constants::AxisHeight:
                    itemToFit->position_[2] += iib->height_;
                    break;
                };
                const auto &GetIntersectCachePBResult = intersectPosDimCaching_.find(itemToFit->position_);
                if (GetIntersectCachePBResult != this->intersectPosDimCaching_.end())
                {
                    if (itemToFit->width_ >= GetIntersectCachePBResult->second[0] && itemToFit->depth_ >= GetIntersectCachePBResult->second[1] && itemToFit->height_ >= GetIntersectCachePBResult->second[2])
                    {
                        continue;
                    };
                };

                if (this->PlaceItemInBin(itemToFitKey))
                {
                    fitted = 1;
                    this->UpdateWithNewFittedItem(itemToFitKey, axis);
                    break;
                };
            };
            if (fitted)
            {
                break;
            };
        };
        if (!fitted)
        {
            unfittedItems_.push_back(itemToFitKey);
        };
    };

    bool PlaceItemInBin(int &it)
    {

        bool R3ItemIntersection, noRotationWillMakeItemFit = 0;
        bool gravityFit = passedOnMasterGravity_->gravityEnabled_;

        Item *itemBeingPlaced = &passedOnMasterItemRegister_->getItem(it);
        const double itemBeingPlacedXPos = itemBeingPlaced->position_[0];
        const double itemBeingPlacedYPos = itemBeingPlaced->position_[1];
        const double itemBeingPlacedZPos = itemBeingPlaced->position_[2];

        std::vector<int> intersectRiskItemKeys;
        std::vector<Item *> intersectRiskItemPtrs;

        const double itemMaxDimension = std::max(std::max(itemBeingPlaced->width_, itemBeingPlaced->depth_), itemBeingPlaced->height_);

        kdTree2_->getPotentialIntersectingItemKeys(kdTree2_->getRoot(),
                                                   kdTree2_->getRoot()->myDepth_,
                                                   itemBeingPlaced->position_,
                                                   {placedItemsMaxDimensions_[0] + itemMaxDimension,
                                                    placedItemsMaxDimensions_[1] + itemMaxDimension,
                                                    placedItemsMaxDimensions_[2] + itemMaxDimension},
                                                   intersectRiskItemKeys);

        for (int i = intersectRiskItemKeys.size(); i--;)
        {
            intersectRiskItemPtrs.push_back(&passedOnMasterItemRegister_->getItem(intersectRiskItemKeys[i]));
        };

        // iter over item allowed rotations
        for (int stringCharCounter = 0; stringCharCounter < itemBeingPlaced->allowedRotations_.size(); stringCharCounter++)
        {
            itemBeingPlaced->SetNewItemDimensions(itemBeingPlaced->allowedRotations_[stringCharCounter] - '0');

            if (width_ < itemBeingPlacedXPos + itemBeingPlaced->width_ ||
                depth_ < itemBeingPlacedYPos + itemBeingPlaced->depth_ ||
                height_ < itemBeingPlacedZPos + itemBeingPlaced->height_)
            {
                continue;
            };

            for (auto &intersectCandidate : intersectRiskItemPtrs)
            {

                if (itemBeingPlacedXPos >= intersectCandidate->ipwf_ || itemBeingPlacedYPos >= intersectCandidate->ipdf_ || itemBeingPlacedXPos + itemBeingPlaced->width_ <= intersectCandidate->position_[0] || itemBeingPlacedYPos + itemBeingPlaced->depth_ <= intersectCandidate->position_[1])
                {
                    continue;
                };

                /* check for Z intersection, if there is intersection store some caching and stop this item iteration. */
                if (!(intersectCandidate->iphf_ <= itemBeingPlacedZPos || intersectCandidate->position_[2] >= (itemBeingPlacedZPos + itemBeingPlaced->height_)))
                {

                    /* checks if point is in cube, if so then no rotation will help fit and the item position is invalid no matter how we rotate */
                    noRotationWillMakeItemFit = (intersectCandidate->position_[0] <= itemBeingPlacedXPos && itemBeingPlacedXPos <= intersectCandidate->ipwf_ &&
                                                 intersectCandidate->position_[1] <= itemBeingPlacedYPos && itemBeingPlacedYPos <= intersectCandidate->ipdf_ &&
                                                 intersectCandidate->position_[2] <= itemBeingPlacedZPos && itemBeingPlacedZPos <= intersectCandidate->iphf_);

                    /* Cache the smallest possible dimensions for this intersection */
                    const double intersectCacheW = intersectCandidate->position_[0] - itemBeingPlacedXPos;
                    const double intersectCacheD = intersectCandidate->position_[1] - itemBeingPlacedYPos;
                    const double intersectCacheH = intersectCandidate->position_[2] - itemBeingPlacedZPos;

                    const auto &GetIntersectCacheResult = intersectPosDimCaching_.find(itemBeingPlaced->position_);
                    if (GetIntersectCacheResult == intersectPosDimCaching_.end())
                    {
                        intersectPosDimCaching_[itemBeingPlaced->position_] = {intersectCacheW, intersectCacheD, intersectCacheH};
                    }
                    else
                    {
                        intersectPosDimCaching_[itemBeingPlaced->position_] = {
                            std::min(GetIntersectCacheResult->second[0], intersectCacheW),
                            std::min(GetIntersectCacheResult->second[1], intersectCacheD),
                            std::min(GetIntersectCacheResult->second[2], intersectCacheH)};
                    };
                    R3ItemIntersection = 1;
                    goto endIntersectionCheck;
                };
            };

        endIntersectionCheck:
            if (noRotationWillMakeItemFit) // no rotation will help fit the item.
            {
                break;
            };
            if (R3ItemIntersection) // intersection found, rotating might help.
            {
                R3ItemIntersection = 0;
                continue;
            };

            if (!gravityFit) // gravity is enabled.
            {
                if (0.0 == itemBeingPlacedZPos)
                {
                    gravityFit = 1;
                }
                else
                {
                    double totalCoveredSurfaceAreaPercentage = 0.0;
                    for (auto &itemInSpace : GetFittedItems())
                    {
                        Item *iis = &passedOnMasterItemRegister_->getItem(itemInSpace);
                        if (gravityFit)
                        {
                            break;
                        };
                        if (iis->iphf_ != itemBeingPlacedZPos)
                        {
                            continue;
                        };
                        if (!(iis->ipwf_ <= itemBeingPlacedXPos ||
                              (itemBeingPlacedXPos + itemBeingPlaced->width_) <= iis->position_[0] ||
                              iis->ipdf_ <= itemBeingPlacedYPos ||
                              (itemBeingPlacedYPos + itemBeingPlaced->depth_) <= iis->position_[1]))
                        {

                            totalCoveredSurfaceAreaPercentage += std::max(0.0, std::min(iis->ipwf_, (itemBeingPlacedXPos + itemBeingPlaced->width_)) - std::max(iis->position_[0], itemBeingPlacedXPos)) *
                                                                 std::max(0.0, std::min(iis->ipdf_, (itemBeingPlacedYPos + itemBeingPlaced->depth_)) - std::max(iis->position_[1], itemBeingPlacedYPos)) / (itemBeingPlaced->width_ * itemBeingPlaced->depth_) * 100;

                            gravityFit = totalCoveredSurfaceAreaPercentage >= passedOnMasterGravity_->gravityStrengthPercentage_;
                        };
                    };
                }
            };
            if (!gravityFit)
            {
                continue;
            };

            /* If this point is reached then fit=true, set used rotation type and return true to place item in bin */
            itemBeingPlaced->rotationType_ = itemBeingPlaced->allowedRotations_[stringCharCounter] - '0';
            passedOnMasterItemRegister_->getItem(it).SetRotationTypeDesc();
            passedOnMasterItemRegister_->getItem(it).SetItemDimensionInfo();

            return 1;
        };

        /* If this point is reached then fit=false, restore item to original dimensions */
        itemBeingPlaced->SetNewItemDimensions(99);
        itemBeingPlaced->position_ = constants::START_POSITION;

        return 0;
    };
};

#endif