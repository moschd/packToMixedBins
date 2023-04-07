#ifndef ITEM_POSITION_CONSTRUCTOR_H
#define ITEM_POSITION_CONSTRUCTOR_H

#define BASE_ITEM_KEY 0

#include "2dConstructor/heuristicAlgorithms.h"
#include "2dConstructor/rect.h"
#include "2dConstructor/neatPacker/packingAction.h"
#include "2dConstructor/neatPacker/neatPacker.h"
#include "2dConstructor/spiralPacker.h"
#include "2dConstructor/maxRectsBinPack.h"
#include "2dConstructor/algorithmHandler.h"
#include "2dConstructor/geometricShape2D.h"
#include "2dConstructor/item2D.h"
#include "2dConstructor/packingContext2D.h"
#include "2dConstructor/packingLayer.h"
#include "2dConstructor/bin2D.h"

class ItemPositionConstructor
{
private:
    std::vector<int> items_;
    std::map<int, std::vector<int>> distinctItems_;
    std::shared_ptr<PackingContext> context_;
    std::shared_ptr<Bin2D> precalculatedBin_;
    int heightAddition_;
    bool hasPrecalculatedBinAvailable_;
    double minimumSurfaceArea_ = 75.0;

    /**
     * @brief Create a vector of arrays.
     *
     * The inner arrays hold a item with the unique dimensions, and the nr of occurence.
     *
     */
    void filterDistinctItems()
    {

        for (const int &itemKey : ItemPositionConstructor::items_)
        {

            if (ItemPositionConstructor::distinctItems_.empty())
            {
                ItemPositionConstructor::distinctItems_[itemKey] = {itemKey};
            }
            else
            {
                bool isDistinct = true;
                for (std::map<int, std::vector<int>>::iterator mapIterator = distinctItems_.begin(); mapIterator != distinctItems_.end(); ++mapIterator)
                {
                    if (ItemPositionConstructor::context_->itemsAreEqual(itemKey, mapIterator->first))
                    {
                        isDistinct = false;
                        mapIterator->second.push_back(itemKey);
                        break;
                    };
                }
                if (isDistinct)
                {
                    ItemPositionConstructor::distinctItems_[itemKey] = {itemKey};
                }
            }
        }
    }

    /**
     * @brief Return the percentage for which the item could cover the bin.
     * This is purely done based on quantities. It is not checking if it can be packed.
     *
     * @param aItemSurface
     * @param aQuantity
     * @return true
     * @return false
     */
    const double itemSurfaceCoverage(const double aItemSurface, const int aQuantity)
    {
        return (aItemSurface * aQuantity) / ItemPositionConstructor::context_->getRequestedBin()->getRealBottomSurfaceArea() * 100;
    }

    void process()
    {
        int winningSurfaceArea = 0;
        ItemPositionConstructor::hasPrecalculatedBinAvailable_ = false;

        std::shared_ptr<RequestedBin2D> requestedBin2D = std::make_shared<RequestedBin2D>(ItemPositionConstructor::context_->getRequestedBin()->getType(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getWidth(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getDepth(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getHeight(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getMaxWeight(),
                                                                                          0,
                                                                                          0,
                                                                                          0);

        for (std::map<int, std::vector<int>>::iterator distinctItemInfo = distinctItems_.begin(); distinctItemInfo != distinctItems_.end(); ++distinctItemInfo)
        {
            Item2D baseItem(BASE_ITEM_KEY,
                            std::to_string(ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->transientSysId_),
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->width_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->depth_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->height_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->weight_,
                            0,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->getRealVolume());

            const double itemSurfaceArea = baseItem.getReal2DSurfaceArea();
            const int availableItems = (int)distinctItemInfo->second.size();

            // Filters without checking if a good layer can be build. Comparison based on percentage.
            if (minimumSurfaceArea_ > ItemPositionConstructor::itemSurfaceCoverage(itemSurfaceArea, availableItems))
            {
                continue;
            };

            std::shared_ptr<Bin2D> new2DBin = std::make_shared<Bin2D>(std::make_shared<PackingContext2D>(std::make_shared<ItemRegister2D>(baseItem), requestedBin2D));
            new2DBin->startPacking();

            const int itemsThatWillBePlaced = std::min(availableItems, (int)new2DBin->getBaseLayer()->getFittedItems().size());
            const double realSurfaceCoverage = ItemPositionConstructor::itemSurfaceCoverage(itemSurfaceArea, itemsThatWillBePlaced);

            // Layer has now been build.
            // Continue if the layer is not more efficient than the minimum.
            // Continue if the layer is not more efficient than an already found layer.
            if (minimumSurfaceArea_ > realSurfaceCoverage || winningSurfaceArea > realSurfaceCoverage)
            {
                continue;
            }

            winningSurfaceArea = realSurfaceCoverage;
            ItemPositionConstructor::hasPrecalculatedBinAvailable_ = true;
            ItemPositionConstructor::precalculatedBin_ = new2DBin;
        }
    }

public:
    ItemPositionConstructor(std::shared_ptr<PackingContext> aContext,
                            const std::vector<int> aItems) : context_(aContext),
                                                             items_(aItems),
                                                             hasPrecalculatedBinAvailable_(false),
                                                             heightAddition_(0)
    {
        ItemPositionConstructor::reconfigure(aItems);
    };

    /// @brief True if there are precalculated items available for packing.
    /// @return const bool
    const bool hasPrecalculatedBinAvailable() const { return hasPrecalculatedBinAvailable_; };

    /// @brief Get the bin which has been precalculated.
    /// @return const std::shared_ptr<Bin2D>
    const std::shared_ptr<Bin2D> getPrecalculatedBin() const { return precalculatedBin_; };

    /// @brief Get the incremental height additions for subsequent bins.
    /// @return int
    const int getHeightAddition() const { return ItemPositionConstructor::heightAddition_; };

    /// @brief Add to the additional height.
    /// @param aAddition
    void addToHeightAddition(int aAddition) { ItemPositionConstructor::heightAddition_ += aAddition; };

    /// @brief Returns the distinct item key for this packing.
    /// @return const int
    const int getDistinctItemKey() const { return std::atoi(precalculatedBin_->getFittedItems()[0].id_.c_str()); };

    /// @brief Returns the vector of item keys relevant to this distinct item key.
    /// @param aKey
    /// @return const std::vector<int>
    const std::vector<int> getRelevantItems() const { return distinctItems_.at(ItemPositionConstructor::getDistinctItemKey()); };

    /// @brief Get a number of items in the base layer.
    /// @return const int
    const int getNumberOfBaseItems() { return (int)precalculatedBin_->getBaseLayer()->getFittedItems().size(); };

    /// @brief Get a precalculated item by index.
    /// @param aIndex
    /// @return const Item2D
    const Item2D getBaseItemByIndex(const int aIndex)
    {
        const int aItemKey = precalculatedBin_->getBaseLayer()->getFittedItems()[aIndex];
        return precalculatedBin_->getContext()->getItemRegister()->getConstItem(aItemKey);
    };

    void reconfigure(std::vector<int> aItems)
    {
        ItemPositionConstructor::items_ = aItems;
        ItemPositionConstructor::distinctItems_.clear();
        ItemPositionConstructor::filterDistinctItems();
        ItemPositionConstructor::process();
    };
};

#endif
