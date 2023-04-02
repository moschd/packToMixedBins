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
    std::vector<std::array<int, 2>> distinctItems_;
    std::shared_ptr<PackingContext> context_;
    std::shared_ptr<Bin2D> winning2DBin_;

    /**
     * @brief Create a vector of arrays.
     *
     * The inner arrays hold a item with the unique dimensions, and the nr of occurence.
     *
     */
    void filterDistinctItems()
    {
        for (const int &itemKey : items_)
        {

            if (ItemPositionConstructor::distinctItems_.empty())
            {
                ItemPositionConstructor::distinctItems_.push_back({itemKey, 1});
            }
            else
            {

                bool isDistinct = true;
                for (std::array<int, 2> &disinctItems : distinctItems_)
                {
                    if (ItemPositionConstructor::context_->itemsAreEqual(itemKey, disinctItems[0]))
                    {
                        isDistinct = false;
                        disinctItems[1] += 1;
                        break;
                    };
                }
                if (isDistinct)
                {
                    ItemPositionConstructor::distinctItems_.push_back({itemKey, 1});
                }
            }
        }

        // for (std::array<int, 2> disinctItems : distinctItems_)
        // {
        //     std::cout << "Item: " << disinctItems[0] << " occurence " << disinctItems[1] << "\n";
        // };
    }

public:
    bool hasResult_;

    ItemPositionConstructor(std::shared_ptr<PackingContext> aContext,
                            const std::vector<int> aItems) : context_(aContext),
                                                             items_(aItems),
                                                             hasResult_(false)
    {
        ItemPositionConstructor::filterDistinctItems();
        ItemPositionConstructor::process();
    };

    const std::vector<Item2D> getItems() const
    {
        return ItemPositionConstructor::winning2DBin_->getFittedItems();
    }

    void process()
    {

        int winningSurfaceArea = 0;

        for (std::array<int, 2> disinctItem : distinctItems_)
        {
            Item2D baseItem(BASE_ITEM_KEY,
                            ItemPositionConstructor::context_->getItem(disinctItem[0])->id_,
                            ItemPositionConstructor::context_->getItem(disinctItem[0])->width_,
                            ItemPositionConstructor::context_->getItem(disinctItem[0])->depth_,
                            ItemPositionConstructor::context_->getItem(disinctItem[0])->height_,
                            ItemPositionConstructor::context_->getItem(disinctItem[0])->weight_,
                            0);

            std::shared_ptr<ItemRegister2D> itemRegister2D = std::make_shared<ItemRegister2D>(baseItem);
            std::shared_ptr<RequestedBin2D> requestedBin2D = std::make_shared<RequestedBin2D>(ItemPositionConstructor::context_->getRequestedBin()->getType(),
                                                                                              ItemPositionConstructor::context_->getRequestedBin()->getWidth(),
                                                                                              ItemPositionConstructor::context_->getRequestedBin()->getDepth(),
                                                                                              ItemPositionConstructor::context_->getRequestedBin()->getHeight(),
                                                                                              ItemPositionConstructor::context_->getRequestedBin()->getMaxWeight(),
                                                                                              0.0,
                                                                                              0.0,
                                                                                              0.0);

            /// @brief Construct the packing context and initialize the packer which will manage the packing process.
            std::shared_ptr<PackingContext2D> context = std::make_shared<PackingContext2D>(itemRegister2D, requestedBin2D);

            std::shared_ptr<Bin2D> new2DBin = std::make_shared<Bin2D>(context);

            new2DBin->startPacking();

            if (new2DBin->getItemsPerLayer() <= disinctItem[1])
            {
                std::cout << "Nr of items required for base layer " << new2DBin->getItemsPerLayer() << " covered area " << new2DBin->getCoveredSurfaceArea() * MULTIPLIER << "\n";
                if (winningSurfaceArea < int(new2DBin->getCoveredSurfaceArea() * MULTIPLIER))
                {
                    winningSurfaceArea = new2DBin->getCoveredSurfaceArea() * MULTIPLIER;
                    ItemPositionConstructor::winning2DBin_ = new2DBin;
                }
            }
        }

        ItemPositionConstructor::hasResult_ = winningSurfaceArea > 0;
    }
};

#endif
