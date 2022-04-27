#include <iostream>
#include <fstream>

#include <unordered_map>
#include <vector>

#include <jsoncpp/json/json.h>
#include <chrono>
#include <cmath>
#include <regex>

/* 
    ----------------------------------------------------
                        GLOBAL CONSTANTS
    ----------------------------------------------------       
*/

const int AxisWidth = 0;
const int AxisDepth = 1;
const int AxisHeight = 2;

const std::array<int,3> AllAxis = {AxisWidth, AxisDepth, AxisHeight};
const std::array<double,3> START_POSITION = { 0, 0, 0 };

const int RT_WDH = 0;
const int RT_DWH = 1;
const int RT_HDW = 2;
const int RT_DHW = 3;
const int RT_WHD = 4;
const int RT_HWD = 5;

const std::array<int,6> AllRotationTypes = {RT_WDH, RT_DWH, RT_HDW, RT_DHW, RT_WHD, RT_HWD};
const std::regex validAdjustedRotationType("^[0-5]$");





/* 
    ----------------------------------------------------
                        ITEM
    ----------------------------------------------------
                    Unit to be shipped       
*/
class Item {
    public:
        int sysId;
        std::string id;
        double width;
        double depth;
        double height;
        double weight;
        double original_width;
        double original_height;
        double original_depth;
        std::string allowed_rotations;
        int rotation_type;
        std::string item_cons_key;
        std::array<double,3> position;
        std::string rotation_type_description;
        double volume;

        double ipwf;
        double ipdf;
        double iphf;

        Item(int systemId, std::string item_id, double w, double d, double h, double wei, std::string i_cons_key, std::string i_allowed_rotations){
            sysId = systemId;
            id = item_id.size() ? item_id : "NA";
            original_width  = width  = w;
            original_depth  = depth  = d;
            original_height = height = h;
            volume = w * d * h;
            weight = wei;
            item_cons_key = i_cons_key;
            allowed_rotations = i_allowed_rotations.size() ? i_allowed_rotations : "012345";

            rotation_type = 0;
            rotation_type_description = "";
            position = START_POSITION;
        };

    bool operator== (const Item& other) const {
        return sysId == other.sysId ? 1 : 0;
    };

    inline void SetNewItemDimensions(const int reqRotationType) {
        switch (reqRotationType){
            case RT_WDH: // keep {width, depth, height};
                this->width = this->original_width;
                this->depth = this->original_depth;
                this->height = this->original_height;
                break;
            case RT_DWH: // {depth, width, height};
                this->width = this->original_depth;
                this->depth = this->original_width;
                this->height = this->original_height;
                break;
            case RT_HDW: // {height, depth, width};
                this->width = this->original_height;
                this->depth = this->original_depth;
                this->height = this->original_width;
                break;
            case RT_DHW: // {depth, height, width};
                this->width = this->original_depth;
                this->depth = this->original_height;
                this->height = this->original_width;
                break;
            case RT_HWD: // {height, width, depth};
                this->width = this->original_height;
                this->depth = this->original_width;
                this->height = this->original_depth;
                break;
            case RT_WHD: // {width, height, depth};
                this->width = this->original_width;
                this->depth = this->original_height;
                this->height = this->original_depth;
                break;
            default:
                this->width = this->original_width;
                this->depth = this->original_depth;
                this->height = this->original_height;
        };
    };

    inline void SetItemDimensionInfo(){
        this->ipwf = (this->position[0] + this->width);
        this->ipdf = (this->position[1] + this->depth);
        this->iphf = (this->position[2] + this->height);
    };

    void SetRotationTypeDesc(){
        switch (this->rotation_type){
            case RT_WDH:
                this->rotation_type_description = "No box rotation";
                break;
            case RT_DWH:
                this->rotation_type_description = "Rotate the box around the z-axis by 90°";
                break;
            case RT_HDW:
                this->rotation_type_description = "Rotate the box around the x-axis by 90°";
                break;
            case RT_DHW:
                this->rotation_type_description = "Rotate the box around the x-axis by 90° and then around the z-axis by 90°";
                break;
            case RT_WHD:
                this->rotation_type_description = "Rotate the box around the y-axis by 90°";
                break;
            case RT_HWD:
                this->rotation_type_description = "Rotate the box around the z-axis by 90° and then around the x-axis by 90°";
                break;
        };
    };

    void printMe(){
        std::cout << this->id << " p: ";
        for(auto pos : this->position){
            std::cout << pos << " ";
        };
        std::cout << " d: " << width << " " << depth << " " << height;
    };
};



class Gravity{
/*
    GRAVITY.
        Handles logic related to gravity.

    - Boxes being stacked in the air can happen since depth stacking is checked on all items before height stacking.
    - Boxes resting partly on a box.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/
    public:
        bool gravityEnabled;
        double gravityStrengthPercentage;
    
        Gravity(double rGravityStrengthPercentage){
            this->gravityStrengthPercentage = rGravityStrengthPercentage;
            this->gravityEnabled = (rGravityStrengthPercentage > 0.0 ? 0 : 1);
        };
        Gravity(){
            this->gravityStrengthPercentage = 0.0;
        }
        double R2XYItemSurfaceAreaCoveragePercentage(const double floor, const std::vector<Item>& itemsInSpace, const Item& itemToBeChecked){
            if(floor == itemToBeChecked.position[2]) { return 100.0; };
            
            double totalCoveredSurfaceAreaPercentage = 0.0;
            for(auto& itemInSpace : itemsInSpace){
                if(itemInSpace.iphf != itemToBeChecked.position[2]) { continue; };
                if( !(itemInSpace.ipwf <= itemToBeChecked.position[0] || 
                    (itemToBeChecked.position[0] + itemToBeChecked.width) <= itemInSpace.position[0] || 
                    itemInSpace.ipdf <= itemToBeChecked.position[1] || 
                    (itemToBeChecked.position[1] + itemToBeChecked.depth) <= itemInSpace.position[1])) {

                        //calculate the surface area that this box is covering on the box to be placed
                        totalCoveredSurfaceAreaPercentage +=    std::max(0.0, std::min(itemInSpace.ipwf,        (itemToBeChecked.position[0] + itemToBeChecked.width)) 
                                                                            - std::max(itemInSpace.position[0], itemToBeChecked.position[0])) *
                                                                std::max(0.0, std::min(itemInSpace.ipdf,        (itemToBeChecked.position[1] + itemToBeChecked.depth)) 
                                                                            - std::max(itemInSpace.position[1], itemToBeChecked.position[1])) 
                                                                / (itemToBeChecked.width*itemToBeChecked.depth) * 100; 

                        if(totalCoveredSurfaceAreaPercentage > gravityStrengthPercentage) { break; };
                    };
                };
            return totalCoveredSurfaceAreaPercentage;        
        };

        inline double R2XYItemSurfaceAreaCoveragePercentage(const double floor, const Item& itemInSpace, const Item& itemToBeChecked){
            if(floor == itemToBeChecked.position[2]) { return 100.0; };

            return  std::max(0.0, std::min(itemInSpace.ipwf, (itemToBeChecked.position[0] + itemToBeChecked.width)) 
                                - std::max(itemInSpace.position[0], itemToBeChecked.position[0])) *
                    std::max(0.0, std::min(itemInSpace.ipdf, (itemToBeChecked.position[1] + itemToBeChecked.depth)) 
                                - std::max(itemInSpace.position[1], itemToBeChecked.position[1])) 
                    / (itemToBeChecked.width*itemToBeChecked.depth) * 100;        
        };
};


struct itemPositionHashFunction{
    size_t operator()(const std::array<double,3>& itemPos) const{
    size_t h1 = std::hash<double>()(itemPos[0]);
    size_t h2 = std::hash<double>()(itemPos[1]);
    size_t h3 = std::hash<double>()(itemPos[2]);
    return (h1 ^ (h2 << 1)) ^ (h3 << 2);
    }
};

struct itemPositionEqualsFunction{
  bool operator()( const std::array<double,3>& item1Pos, const std::array<double,3>& item2Pos ) const{
    return (item1Pos[0] == item2Pos[0] && item1Pos[1] == item2Pos[1] && item1Pos[2] == item2Pos[2]);
  }
};

class BinSection {
    private:
        int id;
        std::vector<Item> items;
        std::array<double,3> sectionStart;
        std::array<double,3> sectionEnd;

    public:        
        BinSection(int myId, std::array<double,3> startPos, std::array<double,3> maxPos){
            this->id = myId;
            this->sectionStart = startPos;
            this->sectionEnd = maxPos;
        };
        std::array<double,3> getBinSectionStart(){
            return this->sectionStart;
        };
        std::array<double,3> getBinSectionEnd(){
            return this->sectionEnd;
        };
        void addItem(const Item& i){
            this->items.push_back(i);
        };
        const std::vector<Item>& getItems() {
            return this->items;
        };
        bool isWithinMyR3Perimeters(const Item& itemToCheck){
            return !(this->sectionEnd[0]    <= itemToCheck.position[0] 
                    || (itemToCheck.position[0]+itemToCheck.width)     <= this->sectionStart[0]
                    || this->sectionEnd[1]  <= itemToCheck.position[1] 
                    || (itemToCheck.position[1]+itemToCheck.depth)     <= this->sectionStart[1])
                    && (this->sectionEnd[2] > itemToCheck.position[2] && this->sectionStart[2] < (itemToCheck.position[2]+itemToCheck.height));
        };
        bool isWithinMyR2XYPerimeters(const Item& itemToCheck){
            return !(this->sectionEnd[0]    <= itemToCheck.position[0] 
                    || (itemToCheck.position[0]+itemToCheck.width)     <= this->sectionStart[0]
                    || this->sectionEnd[1]  <= itemToCheck.position[1] 
                    || (itemToCheck.position[1]+itemToCheck.depth)     <= this->sectionStart[1]);
        };
        void printMe(){
            std::cout << this->id << " size: " << this->items.size() << " start: ";
            for(auto st : this->getBinSectionStart()){
                std::cout << st << " ";
            };
            std::cout << " end: ";
            for(auto en : this->getBinSectionEnd()){
                std::cout << en << " ";
            };
            std::cout << "\n";
        };
};

class Partitioner {
    private:
        int partitionLevel;
        int estimatedRequiredBins;
        int numberOfItemsToBePacked;
        std::vector<BinSection> binSections;
        std::array<double,3> masterBinStartingPosition;
        std::array<double,3> masterBinMaxDimensions;

    public:
        Partitioner(int reqEstimatedTotalBinsToBePacked, int reqNumberOfItemsToBePacked, std::array<double,3> startPos, std::array<double,3> maxPos) {
            this->masterBinStartingPosition = startPos;
            this->masterBinMaxDimensions = maxPos;
            this->estimatedRequiredBins = reqEstimatedTotalBinsToBePacked;
            this->numberOfItemsToBePacked = reqNumberOfItemsToBePacked;

            BinSection singleSection(this->binSections.size(), this->masterBinStartingPosition, this->masterBinMaxDimensions);
            this->binSections.push_back(singleSection);

            this->calculatePartitionLevel();
            this->generateBinSections();
        };
        std::vector<BinSection>& getBinSections(){
            return this->binSections;
        };
        void calculatePartitionLevel(){
            //the double is the desired number of items per bin section
            this->partitionLevel = ceil(sqrt(((this->numberOfItemsToBePacked/this->estimatedRequiredBins)/750.0)));
        };
       void singlePartition(std::array<double,3> startPos, std::array<double,3> maxPos, int axis){
            double partitionBorder = maxPos[axis] - abs(startPos[axis]-maxPos[axis]) / 2;

            std::array<double,3> part1 = maxPos;
            part1[axis] = partitionBorder;
            BinSection newSection1(this->binSections.size(), startPos, part1);
            this->binSections.push_back(newSection1);

            std::array<double,3> part2 = startPos;
            part2[axis] = partitionBorder;
            BinSection newSection2(this->binSections.size(), part2, maxPos);
            this->binSections.push_back(newSection2);
        };
        void partitionEachBinSection(int axis){
            std::vector<BinSection> cbins = this->binSections;
            this->binSections.clear();
            for(auto& binS : cbins){
                this->singlePartition(binS.getBinSectionStart(),binS.getBinSectionEnd(), axis);
            };
        };
        void generateBinSections(){
            if(this->partitionLevel == 1) { return; };

            int PartitionAxis = AxisWidth;
            for(int i=0; i < this->partitionLevel; i++){
                this->partitionEachBinSection(PartitionAxis);
                switch(PartitionAxis){
                    case AxisWidth:
                        PartitionAxis = AxisDepth;
                        break;
                    case AxisDepth:
                        PartitionAxis = AxisHeight;
                        break;
                    case AxisHeight:
                        PartitionAxis = AxisWidth;
                        break;
                };
            };
        };
};

/* 
    ----------------------------------------------------
                        BIN
    ----------------------------------------------------
                    Stores items       
*/

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

            Partitioner binPartition(estimatedRequiredBins, numberOfItemsToBePacked, START_POSITION, { this->width, this->depth, this->height });
            mySections = binPartition.getBinSections();

            Gravity requestedGravity(rGravityStrength);
            gravityComponent = requestedGravity;

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

        this->act_weight_utilization += it->weight;
        this->act_volume_utilization += it->volume;
        this->x_free_items.push_back(it);
        this->y_free_items.push_back(it);

        auto hiter = std::upper_bound(this->z_free_items.cbegin(), this->z_free_items.cend(), it, [&](const Item* i1, const Item* i2) { return i1->iphf < i2->iphf; });
        this->z_free_items.insert(hiter, it);

        if(ax == AxisHeight){
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
        for(const auto& axis : AllAxis){
            switch (axis){
                case AxisWidth:
                    items_in_bin = this->x_free_items;
                    break;
                case AxisDepth:
                    items_in_bin = this->y_free_items;
                    break;
                case AxisHeight:
                    items_in_bin = this->z_free_items;
                    break;
            };

            for(const auto& item_in_bin : items_in_bin){
                item_to_fit.position = item_in_bin->position;
                switch (axis){
                    case AxisWidth:
                        item_to_fit.position[0] += item_in_bin->width;
                        break;
                    case AxisDepth:
                        item_to_fit.position[1] += item_in_bin->depth;
                        break;
                    case AxisHeight:
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
    it.position = START_POSITION;
 
    return 0;
    };
};


/* 
    ----------------------------------------------------
                        PACKER
    ----------------------------------------------------
                    Packs bins        
*/
class Packer {
    private:
        std::vector<Bin> bins;
        std::vector<std::vector<Item>> FinalSortedItemConsKeyVectors;
        bool includeItems;
        bool itemDimensionsAfter;
        bool includeBins;
        int estimatedTotalRequiredBins;

    public:
        std::vector<Item> items;
        std::string requestedBinType;
        double requestedBinWidth;
        double requestedBinDepth;
        double requestedBinHeight;
        double requestedBinMaxWeight;
        double requestedBinMaxVolume;
        double requestedGravityStrength;
        bool biggestFirst;


        Packer(std::string rBinType, double rBinWidth, double rBinDepth, double rBinHeight, double rBinMaxWeight, 
                bool rBiggestFirst, bool rIncludeBins, bool rIncludeItems, bool rItemDimensionsAfter, double rGravityStrength){ 
            requestedBinType        = rBinType;
            requestedBinWidth       = rBinWidth;
            requestedBinDepth       = rBinDepth;
            requestedBinHeight      = rBinHeight;
            requestedBinMaxWeight   = rBinMaxWeight;
            requestedBinMaxVolume   = (rBinWidth * rBinHeight * rBinDepth);
            biggestFirst            = rBiggestFirst;
            includeBins             = rIncludeBins;
            includeItems            = rIncludeItems;
            itemDimensionsAfter     = rItemDimensionsAfter;
            requestedGravityStrength= rGravityStrength;
        };

        bool GetIncludeBins() { 
            return this->includeBins; 
        };
        std::vector<std::vector<Item>>& GetItemConsKeyVectorsToBePacked() { 
            return this->FinalSortedItemConsKeyVectors; 
        };
        Bin& GetLastBin() { 
            return this->bins.back();
        };
        std::vector<Bin>& GetPackedBinVector() { 
            return this->bins; 
        };

        double GetTotalVolumeUtilizationPercentage(){
            double actualVolumeUtil = 0;
            for(auto& b : this->bins){
                actualVolumeUtil += b.GetActVolumeUtil();
            };
            return actualVolumeUtil / (requestedBinWidth*requestedBinDepth*requestedBinHeight * this->bins.size()) * 100;
        };

        double GetTotalWeightUtilizationPercentage(){
            double actualWeightUtil = 0;
            for(auto& b : this->bins){
                actualWeightUtil += b.GetActWeightUtil();
            };
            return actualWeightUtil / (this->requestedBinMaxWeight * this->bins.size()) * 100;
        };

        Json::Value MapItemToJSON(const Item& item){
            Json::Value JsonItem;
            JsonItem["id"] = item.id;
            JsonItem["width"]   = (this->itemDimensionsAfter ? item.width   : item.original_width);
            JsonItem["depth"]   = (this->itemDimensionsAfter ? item.depth   : item.original_depth);
            JsonItem["height"]  = (this->itemDimensionsAfter ? item.height  : item.original_height);
            JsonItem["weight"] = item.weight;
            JsonItem["volume"] = item.volume;
            if(item.item_cons_key.size()){
                JsonItem["itemConsKey"] = item.item_cons_key;
            };
            JsonItem["allowedRotations"] = item.allowed_rotations;
            JsonItem["xCoordinate"] = item.position[0];
            JsonItem["yCoordinate"] = item.position[1];
            JsonItem["zCoordinate"] = item.position[2];

            if(std::regex_match(std::to_string(item.rotation_type), validAdjustedRotationType)){
                JsonItem["rotationType"] = item.rotation_type;
            };
            if(item.rotation_type_description.size()){
                JsonItem["rotationTypeDescription"] = item.rotation_type_description;
            };
            return JsonItem;
        };

        Json::Value MapBinToJSON(Bin& bin){
            Json::Value mappedBin;
            mappedBin["binName"] = bin.name;
            mappedBin["nrOfItems"] = int(bin.GetFittedItems().size());
            mappedBin["actualVolume"] = bin.GetActVolumeUtil();
            mappedBin["actualVolumeUtil"] = bin.GetActVolumeUtilizationPercentage();
            mappedBin["actualWeight"] = bin.GetActWeightUtil();
            mappedBin["actualWeightUtil"] = bin.GetActWeightUtilizationPercentage();
            if(this->includeItems){
                for(auto& it : bin.GetFittedItems()){
                    mappedBin["fittedItems"].append(this->MapItemToJSON(it));
                };
            };
            return mappedBin;
        };

        void CreateFinalSortedItemConsKeyVectorsAndEstimateBins(){
            /* 
                1.   Sorts items based on item consolidation key and created separate stacking input std::vectors.
                1.1. Sorts the std::vectors based on volume.
                2.   includes the estimated required total bins calculation so i dont have to loop over the items again in a separate function.
            */
            std::sort(this->items.begin(), this->items.end(), [&](const Item& i1, const Item& i2){ return i1.item_cons_key > i2.item_cons_key; });
            double summedTotalItemVolume = 0;
            
            this->FinalSortedItemConsKeyVectors.push_back(std::vector<Item> {this->items[0]} );
            for(int idx = 1; idx < this->items.size(); idx++){
                summedTotalItemVolume += this->items[idx].volume;

                if(this->items[idx].item_cons_key == this->FinalSortedItemConsKeyVectors.back().back().item_cons_key){
                    this->FinalSortedItemConsKeyVectors.back().push_back(this->items[idx]);
                } else {
                    this->FinalSortedItemConsKeyVectors.push_back(std::vector<Item> {this->items[idx]});
                };
            };

            if(this->biggestFirst){
                for(auto& v : this->FinalSortedItemConsKeyVectors){
                    std::sort(v.begin(), v.end(), [&](const Item& i1, const Item& i2){ return i1.volume > i2.volume; } );
                };
            };
            this->estimatedTotalRequiredBins = ceil(summedTotalItemVolume / (this->requestedBinWidth * this-> requestedBinDepth * this->requestedBinHeight));
        };

        void startPacking(std::vector<Item> itemsToBePacked){
            if(itemsToBePacked.empty()){ return; };
            
            Bin new_bin(this->requestedBinType + "-" + std::to_string(this->bins.size()+1),
                        this->requestedBinWidth,
                        this->requestedBinDepth,
                        this->requestedBinHeight,
                        this->requestedBinMaxWeight,
                        this->estimatedTotalRequiredBins,
                        itemsToBePacked.size(),
                        this->requestedGravityStrength);

            this->bins.push_back(new_bin);

            for(auto& item_to_pack : itemsToBePacked){
                if(this->bins.back().GetFittedItems().empty()){ 
                    if(this->bins.back().PlaceItemInBin(item_to_pack)){ 
                        this->bins.back().UpdateWithNewFittedItem(&item_to_pack, -1);
                        continue;
                    };
                };

                if(!this->bins.back().GetUnfittedItems().empty() && 
                    this->bins.back().GetUnfittedItems().back().width == item_to_pack.width && 
                    this->bins.back().GetUnfittedItems().back().height == item_to_pack.height && 
                    this->bins.back().GetUnfittedItems().back().depth == item_to_pack.depth){
                    this->bins.back().GetUnfittedItems().push_back(item_to_pack);
                    continue;
                };

                if((this->bins.back().GetActVolumeUtil() + item_to_pack.volume) >= this->bins.back().max_volume ||
                    (this->bins.back().GetActWeightUtil() + item_to_pack.weight) >= this->bins.back().max_weight){
                    this->bins.back().GetUnfittedItems().push_back(item_to_pack);
                    continue;
                };
                this->bins.back().FindItemPosition(item_to_pack);
            };
            
            if(itemsToBePacked.size() == this->bins.back().GetUnfittedItems().size()){ 
                this->GetPackedBinVector().pop_back();
                return;
            };

            startPacking(this->bins.back().GetUnfittedItems());      
        };
};


/* 
    ----------------------------------------------------
                        INPUT READER
    ----------------------------------------------------
        Reads JSON input, contains ITEMS and BIN        
*/
// extern "C" {
// const char * packToBinAlgorhitm(char* incomingJson, bool biggestFirst=1,
//                                 bool includeBins=1, bool includeItems=1, 
//                                 bool itemDimensionsAfter=0, int jsonPrecision=5,
//                                 double gravityStrength=0.0) {

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    bool biggestFirst           = 1;
    bool includeBins            = 1;
    bool includeItems           = 1;
    bool itemDimensionsAfter    = 0;
    int jsonPrecision           = 5;
    double gravityStrength      = 0.0;

    std::ifstream incomingJson ("./cpp_vs_python_5000_items.json");
    Json::Reader reader;
    Json::Value inboundRoot;
    reader.parse(incomingJson, inboundRoot);
    Json::Value ijb = inboundRoot["bin"];
    Json::Value iji = inboundRoot["items"];
    
    //init Packer
    Packer PackingProcessor(ijb["type"].asString(),
                            ijb["width"].asDouble(),
                            ijb["depth"].asDouble(),
                            ijb["height"].asDouble(),
                            ijb["maxWeight"].asDouble(),
                            biggestFirst,
                            includeBins,
                            includeItems,
                            itemDimensionsAfter,
                            gravityStrength
                            );

    //create item objects
    for (int x=iji.size(); x--;) {
        PackingProcessor.items.push_back(
                                        Item(x,
                                            iji[x]["id"].asString(),
                                            iji[x]["width"].asDouble(),
                                            iji[x]["depth"].asDouble(),
                                            iji[x]["height"].asDouble(),
                                            iji[x]["weight"].asDouble(),
                                            iji[x]["itemConsKey"].asString(),
                                            iji[x]["allowedRotations"].asString()));
    };

    PackingProcessor.CreateFinalSortedItemConsKeyVectorsAndEstimateBins();

    for(auto& sortedItemConsKeyVector : PackingProcessor.GetItemConsKeyVectorsToBePacked()){
        PackingProcessor.startPacking(sortedItemConsKeyVector);
    };

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << std::endl;


    /* OUTGOING JSON BUILDER */
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder.settings_["precision"] = jsonPrecision;
    Json::Value outboundRoot; 

    outboundRoot["requiredNrOfBins"]= int(PackingProcessor.GetPackedBinVector().size());
    outboundRoot["totalVolumeUtil"] = PackingProcessor.GetTotalVolumeUtilizationPercentage();
    outboundRoot["totalWeightUtil"] = PackingProcessor.GetTotalWeightUtilizationPercentage();
    outboundRoot["unfittedItems"]   = Json::arrayValue;

    if(!PackingProcessor.GetLastBin().GetUnfittedItems().empty()){
        for(auto& it : PackingProcessor.GetLastBin().GetUnfittedItems()){
            outboundRoot["unfittedItems"].append(PackingProcessor.MapItemToJSON(it));
        };
    };

    if(PackingProcessor.GetIncludeBins()){
        outboundRoot["binDetails"]["maxWidth"]  = PackingProcessor.requestedBinWidth;
        outboundRoot["binDetails"]["maxDepth"]  = PackingProcessor.requestedBinDepth;
        outboundRoot["binDetails"]["maxHeight"] = PackingProcessor.requestedBinHeight;
        outboundRoot["binDetails"]["maxWeight"] = PackingProcessor.requestedBinMaxWeight;
        outboundRoot["binDetails"]["maxVolume"] = PackingProcessor.requestedBinMaxVolume;

        for(auto& bi : PackingProcessor.GetPackedBinVector()){
            outboundRoot["packedBins"].append(PackingProcessor.MapBinToJSON(bi));
            std::cout << bi.name << " size: " << bi.GetFittedItems().size() << " MaxWeight: " << PackingProcessor.requestedBinMaxWeight <<  "\n";
            for(auto& bs : bi.mySections){
                bs.printMe();
            };
        };
    };

    // std::cout << output << "\n";
    std::ofstream myfile;
    myfile.open ("ex1.txt");
    std::string output = Json::writeString(builder, outboundRoot);
    myfile << output;
    myfile.close();

    // return strdup(Json::writeString(builder, outboundRoot).c_str());
    return 0;
// };

};


extern "C" {
const void packToBinAlgorhitmFreeMemory(char *outputPtr) {
    free(outputPtr);
    };
};
