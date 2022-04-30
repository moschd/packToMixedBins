#include <vector>
#include "item.h"

struct itemRegister {

    std::unordered_map<int, Item> completeItemMap;

    void fillRegister(Item item){
        completeItemMap.insert({item.transientSysId, item});
        std::cout << " adding " << item.transientSysId << " " << item.id << " " << item.width << "\n";
        for(auto& item : completeItemMap){
            std::cout << "   " << item.second.id << "\n";
        };

    };

};

