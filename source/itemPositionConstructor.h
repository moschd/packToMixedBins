#ifndef ITEM_POSITION_CONSTRUCTOR_H
#define ITEM_POSITION_CONSTRUCTOR_H

#define MULTIPLIER 10000
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
    std::shared_ptr<Bin2D> winning2DBin_;
    double heightAddition_;

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

    void process()
    {

        ItemPositionConstructor::hasResult_ = false;

        int winningSurfaceArea = 0;

        std::shared_ptr<RequestedBin2D> requestedBin2D = std::make_shared<RequestedBin2D>(ItemPositionConstructor::context_->getRequestedBin()->getType(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getWidth(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getDepth(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getHeight(),
                                                                                          ItemPositionConstructor::context_->getRequestedBin()->getMaxWeight(),
                                                                                          0.0,
                                                                                          0.0,
                                                                                          0.0);

        for (std::map<int, std::vector<int>>::iterator distinctItemInfo = distinctItems_.begin(); distinctItemInfo != distinctItems_.end(); ++distinctItemInfo)
        {
            std::cout << "Going in for distinct item: " << distinctItemInfo->first << "\n";
            Item2D baseItem(BASE_ITEM_KEY,
                            std::to_string(ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->transientSysId_),
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->width_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->depth_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->height_,
                            ItemPositionConstructor::context_->getItem(distinctItemInfo->first)->weight_,
                            0);

            std::shared_ptr<ItemRegister2D> itemRegister2D = std::make_shared<ItemRegister2D>(baseItem);

            /// @brief Construct the packing context and initialize the packer which will manage the packing process.
            std::shared_ptr<Bin2D> new2DBin = std::make_shared<Bin2D>(std::make_shared<PackingContext2D>(itemRegister2D, requestedBin2D));

            new2DBin->startPacking();
            if (new2DBin->getItemsPerLayer() <= (int)distinctItemInfo->second.size() && new2DBin->getItemsPerLayer() > 0)
            {
                std::cout << "Nr of items required for base layer " << new2DBin->getItemsPerLayer() << " covered area " << int(new2DBin->getCoveredSurfaceArea() * MULTIPLIER) << "\n";
                if (winningSurfaceArea < int(new2DBin->getCoveredSurfaceArea() * MULTIPLIER))
                {
                    winningSurfaceArea = int(new2DBin->getCoveredSurfaceArea() * MULTIPLIER);
                    ItemPositionConstructor::winning2DBin_ = new2DBin;
                    ItemPositionConstructor::hasResult_ = true;
                }
            }
        }
    }

public:
    bool hasResult_;

    ItemPositionConstructor(std::shared_ptr<PackingContext> aContext,
                            const std::vector<int> aItems) : context_(aContext),
                                                             items_(aItems),
                                                             hasResult_(false),
                                                             heightAddition_(0)
    {
        ItemPositionConstructor::reconfigure(aItems);
    };

    /**
     * @brief Get the items as calculated by the 2d packer.
     *
     * @return const std::vector<Item2D>
     */
    const std::vector<Item2D> getPrecalculatedItems() const
    {
        return ItemPositionConstructor::winning2DBin_->getFittedItems();
    }

    const int getItemKeyForWhichALayerWasCalculated() const
    {
        return std::stoi(ItemPositionConstructor::getPrecalculatedItems().front().id_);
    }

    const bool hasResult()
    {
        return ItemPositionConstructor::hasResult_;
    };

    const double getHeightAddition() const { return ItemPositionConstructor::heightAddition_; };

    void addToHeightAddition(double aAddition) { ItemPositionConstructor::heightAddition_ += aAddition; };

    /**
     * @brief Get the items which are the same as this single distinct item.
     *
     * @param aKey
     * @return const std::vector<int>
     */
    const std::vector<int> getItemsFromDistinctItem() const
    {
        return ItemPositionConstructor::distinctItems_.at(ItemPositionConstructor::getItemKeyForWhichALayerWasCalculated());
    }

    void reconfigure(std::vector<int> aItems)
    {
        ItemPositionConstructor::items_ = aItems;
        ItemPositionConstructor::distinctItems_.clear();
        ItemPositionConstructor::filterDistinctItems();
        ItemPositionConstructor::process();
    };
};

#endif
