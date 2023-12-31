#ifndef ITEM_POSITION_CONSTRUCTOR_H
#define ITEM_POSITION_CONSTRUCTOR_H

#include "homogenousLayerBuilder/heuristicAlgorithms.h"
#include "homogenousLayerBuilder/rect.h"
#include "homogenousLayerBuilder/packingAction.h"
#include "homogenousLayerBuilder/neatPacker.h"
#include "homogenousLayerBuilder/maxRectsBinPack.h"
#include "homogenousLayerBuilder/algorithmHandler.h"
#include "homogenousLayerBuilder/packingLayer.h"
#include "homogenousLayerBuilder/bin2D.h"

class ItemPositionConstructor
{
private:
    std::vector<int> items_;
    std::map<int, std::vector<int>> distinctItems_;
    std::shared_ptr<PackingContext> context_;
    std::shared_ptr<Bin2D> precalculatedBin_;
    int heightAddition_;
    bool hasPrecalculatedBinAvailable_;
    bool containsItemsWithNoItemsOnTopStackingStyle_;
    double minimumSurfaceArea_ = 74.0;

    /**
     * @brief Create a map of unique items.
     *
     * The key is the key of the first unique item of that kind.
     * The values are then the keys of the items which are equal to the first one.
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
                    if (ItemPositionConstructor::context_->getItemRegister()->itemsAreLooselyEqual(itemKey, mapIterator->first))
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

            ItemPositionConstructor::setContainsItemsWithNoItemsOnTopStackingStyle(itemKey);
        }
    }

    /**
     * @brief Set sontainsItemsWithNoItemsOnTopStackingStyle_
     *
     * If already true, keep true.
     * If false, check the provided item.
     *
     * @param aItemKey
     */
    void setContainsItemsWithNoItemsOnTopStackingStyle(const int aItemKey)
    {

        if (ItemPositionConstructor::containsItemsWithNoItemsOnTopStackingStyle_)
        {
            return;
        }

        ItemPositionConstructor::containsItemsWithNoItemsOnTopStackingStyle_ = (context_->getItemRegister()->getConstItem(aItemKey)->stackingStyle_ == constants::item::parameter::BOTTOM_NO_ITEMS_ON_TOP);
    };

    /**
     * @brief If the calculated layer contains items with a bottomNoItemsUp stacking style, they should be placed first.
     *
     */
    void sortDistinctItems()
    {
        for (std::map<int, std::vector<int>>::iterator distinctItemInfo = distinctItems_.begin(); distinctItemInfo != distinctItems_.end(); ++distinctItemInfo)
        {
            context_->getItemRegister()->moveBottomNoItemsUpToBackOfVector(distinctItemInfo->second);
        };
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

    /**
     * @brief Create a base item based on the item Key.
     *
     * @param aItemKey
     * @return const std::shared_ptr<Item>
     */
    const std::shared_ptr<Item> createBaseItem(const int aItemKey) const
    {
        const std::vector<std::string> compatibleBins;
        return std::make_shared<Item>(BASE_ITEM_KEY,
                                      std::to_string(context_->getItem(aItemKey)->transientSysId_),
                                      context_->getItem(aItemKey)->width_,
                                      context_->getItem(aItemKey)->depth_,
                                      context_->getItem(aItemKey)->height_,
                                      context_->getItem(aItemKey)->weight_,
                                      "none",
                                      "01",
                                      context_->getItem(aItemKey)->gravityStrength_,
                                      compatibleBins,
                                      context_->getItem(aItemKey)->stackingStyle_);
    }

    /**
     * @brief Perform optimized 2D layer packing. Store the precalculated 2D bin if solution is found.
     *
     */
    void process()
    {

        int winningSurfaceArea = 0;

        for (std::map<int, std::vector<int>>::iterator distinctItemInfo = distinctItems_.begin(); distinctItemInfo != distinctItems_.end(); ++distinctItemInfo)
        {

            const std::shared_ptr<Item> baseItem = ItemPositionConstructor::createBaseItem(distinctItemInfo->first);

            if (ItemPositionConstructor::containsItemsWithNoItemsOnTopStackingStyle_)
            {
                if (context_->getItemRegister()->getConstItem(distinctItemInfo->second.back())->stackingStyle_ != constants::item::parameter::BOTTOM_NO_ITEMS_ON_TOP)
                {
                    continue;
                };
            };

            const double itemSurfaceArea = baseItem->getRealBottomSurfaceArea();
            const int availableItems = (int)distinctItemInfo->second.size();

            // Filters without checking if a good layer can be build. Comparison based on percentage.
            if (minimumSurfaceArea_ > ItemPositionConstructor::itemSurfaceCoverage(itemSurfaceArea, availableItems))
            {
                continue;
            };

            // Create fresh item register, and create a 2d bin instance to layer pack.
            std::shared_ptr<ItemRegister> new2DItemRegister = std::make_shared<ItemRegister>(context_->getItemRegister()->getSortMethod(), 10);
            new2DItemRegister->addItem(baseItem);

            std::shared_ptr<Bin2D> new2DBin =
                std::make_shared<Bin2D>(std::make_shared<PackingContext>(context_->getGravity(),
                                                                         new2DItemRegister,
                                                                         context_->getRequestedBin()));
            new2DBin->startPacking();

            // Calculate the efficiency of this newly constructed layer.
            const int itemsThatWillBePlaced = std::min(availableItems, (int)new2DBin->getBaseLayer()->getFittedItems().size());
            const double realSurfaceCoverage = ItemPositionConstructor::itemSurfaceCoverage(itemSurfaceArea, itemsThatWillBePlaced);

            // Layer has now been build.
            // Continue search if the layer is not more efficient than the minimum.
            // Continue search if the layer is not more efficient than an already found layer.
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

    /// @brief Checks if the ItemPositionConstructor contains such an item, if so, it can build 1 layer at max. That one layer can only be of this particular stackingStyle.
    /// @return const bool
    const bool containsItemsWithNoItemsOnTopStackingStyle() const { return containsItemsWithNoItemsOnTopStackingStyle_; };

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
    const int getDistinctItemKey() const { return std::atoi(precalculatedBin_->getFittedItems()[0]->id_.c_str()); };

    /// @brief Returns the vector of item keys relevant to this distinct item key.
    /// @param aKey
    /// @return const std::vector<int>
    const std::vector<int> &getRelevantItems() const { return distinctItems_.at(ItemPositionConstructor::getDistinctItemKey()); };

    /// @brief Get a number of items in the base layer.
    /// @return const int
    const int getNumberOfBaseItems() { return (int)precalculatedBin_->getBaseLayer()->getFittedItems().size(); };

    /// @brief Get a precalculated item by index.
    /// @param aIndex
    /// @return const std::shared_ptr<Item>
    const std::shared_ptr<Item> &getBaseItemByIndex(const int aIndex)
    {
        const int aItemKey = precalculatedBin_->getBaseLayer()->getFittedItems()[aIndex];
        return precalculatedBin_->getContext()->getItemRegister()->getConstItem(aItemKey);
    };

    void reconfigure(std::vector<int> aItems)
    {
        ItemPositionConstructor::items_ = aItems;
        ItemPositionConstructor::hasPrecalculatedBinAvailable_ = false;
        ItemPositionConstructor::containsItemsWithNoItemsOnTopStackingStyle_ = false;
        ItemPositionConstructor::distinctItems_.clear();
        ItemPositionConstructor::filterDistinctItems();
        ItemPositionConstructor::sortDistinctItems();
        ItemPositionConstructor::process();
    };
};

#endif
