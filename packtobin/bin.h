#include "constants.h"
#include "hashfucntions.h"
#include "3r-kd-tree.h"

class Bin {
    private:
        double width;
        double height;
        double depth;
        std::vector<Item> items;
        std::vector<Item> unfitted_items;
        double act_volume_utilization;
        double act_weight_utilization;
        
    public: 
        std::string name;
        double max_weight;
        double max_volume;
        std::vector<Item*> x_free_items;
        std::vector<Item*> y_free_items;
        std::vector<Item*> z_free_items;
        std::vector<BinSection> mySections;
        std::unordered_map<std::array<double,3>, std::array<double,3>, itemPositionHashFunction, itemPositionEqualsFunction> intersectPosDimCaching;
        Gravity gravityComponent;
        Node* kdTree;

        Bin(std::string n, double w, double d, double h, double max_w, int estimatedRequiredBins, 
            int numberOfItemsToBePacked, double rGravityStrength) {
            name = n;
            width = w;
            depth = d;
            height = h;
            max_weight = max_w;
            max_volume = w * d * h;
            act_volume_utilization = 0;
            act_weight_utilization = 0;

            Partitioner binPartition(estimatedRequiredBins, numberOfItemsToBePacked, constants::START_POSITION, { this->width, this->depth, this->height });
            mySections = binPartition.getBinSections();

            Gravity requestedGravity(rGravityStrength);
            gravityComponent = requestedGravity;

            kdTree = init_tree();
        };

    std::vector<Item>& GetFittedItems() { 
        return items; 
    };
    std::vector<Item>& GetUnfittedItems() { 
        return unfitted_items; 
    };
    double GetActVolumeUtilizationPercentage(){
        return this->act_volume_utilization / this->max_volume * 100;
    };
    double GetActWeightUtilizationPercentage(){
        return this->act_weight_utilization / this->max_weight * 100;
    };
    double GetActWeightUtil() { 
        return this->act_weight_utilization; 
    };
    double GetActVolumeUtil() { 
        return this->act_volume_utilization; 
    };
    void AddItemToRelevantBinSections(const Item& itemToPlace){
        for(auto& binS : this->mySections){
            if(binS.isWithinMyR3Perimeters(itemToPlace)){
                binS.addItem(itemToPlace);
            };
        };
    };
    
    void removeFromXFreeItems(const Item* it){
        x_free_items.erase(
                std::remove_if(begin(x_free_items), end(x_free_items), [&](Item* itemInBin) -> bool { 
                        return (it->position[0] == itemInBin->ipwf &&
                            !(it->position[1] >= itemInBin->ipdf && it->ipdf <= itemInBin->position[1]) &&
                            !(it->position[2] >= itemInBin->iphf && it->iphf <= itemInBin->position[2]));
                        }
                    ), end(x_free_items));
    };

    void removeFromYFreeItems(const Item* it){
        y_free_items.erase(
                std::remove_if(begin(y_free_items), end(y_free_items), [&](Item* itemInBin) -> bool { 
                    return (it->position[1] == itemInBin->ipdf && 
                            !(it->position[0] >= itemInBin->ipwf && it->ipwf <= itemInBin->position[0]) &&
                            !(it->position[2] >= itemInBin->iphf && it->iphf <= itemInBin->position[2]));
                        }
                    ), end(y_free_items));
    };
    void removeFromZFreeItems(const Item* it){
        z_free_items.erase(
                std::remove_if(begin(z_free_items), end(z_free_items), [&](Item* itemInBin) -> bool { 
                    return (itemInBin->iphf == it->position[2] &&
                            itemInBin->position[0] <= it->position[0] && it->position[0] <= itemInBin->ipwf &&
                            itemInBin->position[1] <= it->position[1] && it->position[1] <= itemInBin->ipdf);
                            }
                    ), end(z_free_items));
    };

    void UpdateWithNewFittedItem(Item* it, int ax){
        this->items.push_back(*it);
        this->AddItemToRelevantBinSections(*it);
        this->kdTree = this->kdTree->addToTree(this->kdTree, {it->ipwf, it->ipdf, it->iphf}, it->transientSysId);

        this->act_weight_utilization += it->weight;
        this->act_volume_utilization += it->volume;
        this->x_free_items.push_back(it);
        this->y_free_items.push_back(it);

        auto hiter = std::upper_bound(this->z_free_items.cbegin(), this->z_free_items.cend(), it, [&](const Item* i1, const Item* i2) { return i1->iphf < i2->iphf; });
        this->z_free_items.insert(hiter, it);

        if(ax == constants::AxisHeight){
            removeFromZFreeItems(it);
        } else {
            removeFromXFreeItems(it);
            removeFromYFreeItems(it);
            removeFromZFreeItems(it);
        };
    };

    void FindItemPosition(Item& item_to_fit){
        bool fitted = 0;
        std::vector<Item*> items_in_bin;
        for(const auto& axis : constants::AllAxis){
            switch (axis){
                case constants::AxisWidth:
                    items_in_bin = this->x_free_items;
                    break;
                case constants::AxisDepth:
                    items_in_bin = this->y_free_items;
                    break;
                case constants::AxisHeight:
                    items_in_bin = this->z_free_items;
                    break;
            };

            for(const auto& item_in_bin : items_in_bin){
                item_to_fit.position = item_in_bin->position;
                switch (axis){
                    case constants::AxisWidth:
                        item_to_fit.position[0] += item_in_bin->width;
                        break;
                    case constants::AxisDepth:
                        item_to_fit.position[1] += item_in_bin->depth;
                        break;
                    case constants::AxisHeight:
                        item_to_fit.position[2] += item_in_bin->height;
                        break;
                };

                const auto& GetIntersectCachePBResult = this->intersectPosDimCaching.find(item_to_fit.position);
                if (GetIntersectCachePBResult != this->intersectPosDimCaching.end()){
                    if(     item_to_fit.width  >= GetIntersectCachePBResult->second[0] 
                        &&  item_to_fit.depth  >= GetIntersectCachePBResult->second[1] 
                        &&  item_to_fit.height >= GetIntersectCachePBResult->second[2] ){
                        continue;
                    };
                };

                if(this->PlaceItemInBin(item_to_fit)){
                    fitted = 1;
                    this->UpdateWithNewFittedItem(&item_to_fit, axis);
                    break;
                };
            };
            if(fitted){ break; };
        };
        if(!fitted){ 
            this->unfitted_items.push_back(item_to_fit);
        };
    };

    bool PlaceItemInBin(Item& it){
        bool gravityFit = this->gravityComponent.gravityEnabled;
		bool R3ItemIntersection = 0;
		bool noRotationWillMakeItemFit = 0;

        // iter over item allowed rotations
        for(int stringCharCounter = 0; stringCharCounter < it.allowed_rotations.size(); stringCharCounter++){
            it.SetNewItemDimensions(it.allowed_rotations[stringCharCounter] - '0');
            
            if (   (this->width  < (it.position[0] + it.width)) 
                || (this->depth  < (it.position[1] + it.depth)) 
                || (this->height < (it.position[2] + it.height))){ 
                continue; 
            };       

            // iter over bin sections
            for(auto& binS : this->mySections){
                if(!binS.isWithinMyR3Perimeters(it)){ continue; };

                // iter over items in bin section
                const std::vector<Item>& currentItems = binS.getItems();
                for (int i = binS.getItems().size(); i--;){  
                    if(currentItems[i].ipwf   <= it.position[0]  || 
                        (it.position[0] + it.width)      <= currentItems[i].position[0] || 
                        currentItems[i].ipdf  <= it.position[1]  || 
                        (it.position[1] + it.depth)      <= currentItems[i].position[1]) {
                        continue; 
                    };

                    // check for item intersection, if there is intersection stop this iter.
                    if( currentItems[i].iphf > it.position[2] && currentItems[i].position[2] < (it.position[2] + it.height)){

                        //checks if point is in cube, if so then no rotation will help fit and the item position is invalid no matter how we rotate.
                        noRotationWillMakeItemFit = (currentItems[i].position[0] <= it.position[0] <= currentItems[i].ipwf &&
                                                     currentItems[i].position[1] <= it.position[1] <= currentItems[i].ipdf &&
                                                     currentItems[i].position[2] <= it.position[2] <= currentItems[i].iphf);

                        const double cache_w = abs(currentItems[i].position[0] - it.position[0]);
                        const double cache_d = abs(currentItems[i].position[1] - it.position[1]);
                        const double cache_h = abs(currentItems[i].position[2] - it.position[2]);

                        const auto& GetIntersectCacheResult = this->intersectPosDimCaching.find(it.position);  
                        if (GetIntersectCacheResult == this->intersectPosDimCaching.end()){
                            this->intersectPosDimCaching[it.position] = {cache_w, cache_d, cache_h};
                        }
                        else {
                            this->intersectPosDimCaching[it.position] = {
                                std::min(GetIntersectCacheResult->second[0], cache_w),
                                std::min(GetIntersectCacheResult->second[1], cache_d),  
                                std::min(GetIntersectCacheResult->second[2], cache_h)}; 
                        };
                        R3ItemIntersection = 1;
                        goto endIntersectionCheck;

                    } else if(currentItems[i].iphf == it.position[2] && !gravityFit){
                        if(gravityComponent.R2XYItemSurfaceAreaCoveragePercentage(0.0, currentItems[i], it) >= gravityComponent.gravityStrengthPercentage){
                            gravityFit = 1;
                        };
                    };
                };
            };
		
        endIntersectionCheck:
            if(noRotationWillMakeItemFit){ break; };    // no rotation will help fit the item.
            if(R3ItemIntersection){ continue; }         // intersection found, rotating might help

            if(!gravityFit){                            // gravity support is not established yet. iter to find it.
                for(auto& binS : this->mySections){
                    if(!binS.isWithinMyR2XYPerimeters(it)){ continue; };
                    if(gravityComponent.R2XYItemSurfaceAreaCoveragePercentage(0.0, binS.getItems(), it) >= gravityComponent.gravityStrengthPercentage){
                        gravityFit = 1;
                        break;
                    };
                };
                if(!gravityFit){ continue; };
            };
            
            /* If this point is reached then fit=true, set used rotation type and return true to place item in bin */
            it.rotation_type = it.allowed_rotations[stringCharCounter] - '0';
            it.SetRotationTypeDesc();
            it.SetItemDimensionInfo();
            return 1;
        };
    
    /* If this point is reached then fit=false, restore item to original dimensions */
    it.SetNewItemDimensions(99);
    it.position = constants::START_POSITION;
 
    return 0;
    };
};