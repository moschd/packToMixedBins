#ifndef ITEM_REGISTER_H
#define ITEM_REGISTER_H

#include <vector>
#include "item.h"

#include <unordered_map>

struct itemRegister {
    private:
        std::vector<Item> completeItemVector_;
        std::unordered_map<int, Item> completeItemMap_;

    public:
        void fillItemRegisters(Item& item){
            completeItemVector_.push_back(item);
            completeItemMap_.insert({item.transientSysId_, item});
        };

        std::vector<Item>& getCompleteItemVector(){
            return completeItemVector_;
        }

        Item& getItem(int key){
            return completeItemMap_.at(key);
        }

        void updateItemDetails(int key){
            completeItemMap_.at(key).SetRotationTypeDesc();
            completeItemMap_.at(key).SetItemDimensionInfo();
        };
};

#endif