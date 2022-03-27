#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <string.h>
#include <stdio.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <regex>
#include <chrono>

using namespace std;
/* 
    ----------------------------------------------------
                        GLOBAL CONSTANTS
    ----------------------------------------------------       
*/

const int AxisWidth = 0;
const int AxisDepth = 1;
const int AxisHeight = 2;

const array<int,3> AllAxis = {AxisWidth, AxisDepth, AxisHeight};
const array<double,3> START_POSITION = { 0, 0, 0 };

const int RT_WDH = 0;
const int RT_DWH = 1;
const int RT_HDW = 2;
const int RT_DHW = 3;
const int RT_WHD = 4;
const int RT_HWD = 5;

const array<int,6> AllRotationTypes = {RT_WDH, RT_DWH, RT_HDW, RT_DHW, RT_WHD, RT_HWD};
const regex validAdjustedRotationType("^[012345]$");



/* 
    ----------------------------------------------------
                        ITEM
    ----------------------------------------------------
                    Unit to be shipped       
*/
class Item {
    public:
        int sysId;
        string id;
        double width;
        double depth;
        double height;
        double weight;
        double original_width;
        double original_height;
        double original_depth;
        string allowed_rotations;
        int rotation_type;
        string item_cons_key;
        array<double,3> position;
        int adjusted_rotation_type;
        string rotation_type_description;
        double volume;

        double ipwf;
        double ipdf;
        double iphf;
        double pw;
        double pd;
        double ph;

        Item(int systemId, string item_id, double w, double d, double h, double wei, string i_cons_key, string i_allowed_rotations){
            sysId = systemId;
            id = (item_id.size()) ? item_id : "NA";
            width = w;
            depth = d;
            height = h;
            original_width = w;
            original_depth = d;
            original_height = h;
            volume = w * d * h;
            weight = wei;
            item_cons_key = i_cons_key;
            allowed_rotations = (i_allowed_rotations.size() ? i_allowed_rotations : "012345");

            rotation_type = 0;
            rotation_type_description = "";
            position = START_POSITION;
        };

    bool operator== (const Item& other) const {
        return sysId == other.sysId ? 1 : 0;
    };

    array<double,3> GetNewItemDimensions(const int reqRotationType) {
        array<double,3> dims = START_POSITION;
        switch (reqRotationType){
            case RT_WDH:
                dims = {width, depth, height};
                break;
            case RT_DWH:
                dims = {depth, width, height};
                break;
            case RT_HDW:
                dims = {height, depth, width};
                break;
            case RT_DHW:
                dims = {depth, height, width};
                break;
            case RT_HWD:
                dims = {height, width, depth};
                break;
            case RT_WHD:
                dims = {width, height, depth};
                break;
        };
        return dims;
    };

    void SetItemDimensionInfo(){
        this->pw = position[0];
        this->pd = position[1];
        this->ph = position[2];
        this->ipwf = (this->pw + this->width);
        this->ipdf = (this->pd + this->depth);
        this->iphf = (this->ph + this->height);
    };

    void SetRotationTypeDesc(){
        switch (this->adjusted_rotation_type){
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
};

struct itemPositionHashFunction{
    size_t operator()(const array<double,3>& itemPos) const{
    size_t h1 = std::hash<double>()(itemPos[0]);
    size_t h2 = std::hash<double>()(itemPos[1]);
    size_t h3 = std::hash<double>()(itemPos[2]);
    return (h1 ^ (h2 << 1)) ^ (h3 << 2);
    }
};

struct itemPositionEqualsFunction{
  bool operator()( const array<double,3>& item1Pos, const array<double,3>& item2Pos ) const{
    return (item1Pos[0] == item2Pos[0]) && (item1Pos[1] == item2Pos[1]) && (item1Pos[2] == item2Pos[2]);
  }
};

class BinSection {
    private:
        int id;
        vector<Item> items;
        array<double,3> sectionStart;
        array<double,3> sectionEnd;

    public:        
        BinSection(int myId, array<double,3> startPos, array<double,3> maxPos){
            this->id = myId;
            this->sectionStart = startPos;
            this->sectionEnd = maxPos;
        };
        array<double,3> getBinSectionStart(){
            return this->sectionStart;
        };
        array<double,3> getBinSectionEnd(){
            return this->sectionEnd;
        };
        void addItem(const Item& i){
            this->items.push_back(i);
        };
        const vector<Item>& getItems() {
            return this->items;
        };
        int getId(){
            return this->id;
        };
        bool isWithinMyPerimeters(const Item& itemToCheck){
            return !(this->sectionEnd[0]    <= itemToCheck.position[0] 
                    || (itemToCheck.position[0]+itemToCheck.width)     <= this->sectionStart[0]
                    || this->sectionEnd[1]  <= itemToCheck.position[1] 
                    || (itemToCheck.position[1]+itemToCheck.depth)     <= this->sectionStart[1])
                    && (this->sectionEnd[2] > itemToCheck.position[2] && this->sectionStart[2] < (itemToCheck.position[2]+itemToCheck.height));
        };
        bool startsWithinMyPerimeters(const Item& itemToCheck){
            return (   this->sectionStart[0] <= itemToCheck.position[0] <= this->sectionEnd[0] 
                    && this->sectionStart[1] <= itemToCheck.position[1] <= this->sectionEnd[1] 
                    && this->sectionStart[2] <= itemToCheck.position[2] <= this->sectionEnd[2]);
        };
        void printMe(){
            cout << this->id << " size: " << this->getItems().size() << " start: ";
            for(auto st : this->getBinSectionStart()){
                cout << st << " ";
            };
            cout << " end: ";
            for(auto en : this->getBinSectionEnd()){
                cout << en << " ";
            };
            cout << "\n";
        };
};

class Partitioner {
    private:
        int partitionLevel;
        int estimatedRequiredBins;
        int numberOfItemsToBePacked;
        vector<BinSection> binSections;
        array<double,3> masterBinStartingPosition;
        array<double,3> masterBinMaxDimensions;

    public:
        Partitioner(int reqEstimatedTotalBinsToBePacked, int reqNumberOfItemsToBePacked, array<double,3> startPos, array<double,3> maxPos) {
            this->masterBinStartingPosition = startPos;
            this->masterBinMaxDimensions = maxPos;
            this->estimatedRequiredBins = reqEstimatedTotalBinsToBePacked;
            this->numberOfItemsToBePacked = reqNumberOfItemsToBePacked;

            BinSection singleSection(this->binSections.size(), this->masterBinStartingPosition, this->masterBinMaxDimensions);
            this->binSections.push_back(singleSection);

            this->calculatePartitionLevel();
            this->generateBinSections();
        };
        vector<BinSection>& getBinSections(){
            return this->binSections;
        };
        void calculatePartitionLevel(){
            //the double is the desired max number of items per bin section.
            this->partitionLevel = ceil(sqrt(((this->numberOfItemsToBePacked/this->estimatedRequiredBins)/750.0)));
        };
       void singlePartition(array<double,3> startPos, array<double,3> maxPos, int axis){
            double partitionBorder = maxPos[axis] - abs(startPos[axis]-maxPos[axis]) / 2;

            array<double,3> part1 = maxPos;
            part1[axis] = partitionBorder;
            BinSection newSection1(this->binSections.size(), startPos, part1);
            this->binSections.push_back(newSection1);

            array<double,3> part2 = startPos;
            part2[axis] = partitionBorder;
            BinSection newSection2(this->binSections.size(), part2, maxPos);
            this->binSections.push_back(newSection2);
        };
        void partitionEachBinSection(int axis){
            vector<BinSection> cbins = this->binSections;
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
                    ITEM UTIL FUNCTIONS
    ----------------------------------------------------
*/
inline bool sortItemsOnVolume (const Item& i1, const Item& i2) { 
    return (i1.volume > i2.volume); 
};

inline bool sortItemsOnHeight (const Item& i1, const Item& i2) { 
    return i1.iphf < i2.iphf;  
};

inline bool point_in_cuboid(double item_start_x, double item_start_y, double item_start_z, array<double,3> current_item_in_bin_min, double ipwf, double ipdf, double iphf){
    //checks if point is in cube, if so then no rotation will help fit and the item position is invalid.
    return (current_item_in_bin_min[0] <= item_start_x <= ipwf and current_item_in_bin_min[1] <= item_start_y <= ipdf and current_item_in_bin_min[2] <= item_start_z <= iphf);
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
        string name;
        double max_weight;
        double max_volume;
        std::vector<Item> x_free_items;
        std::vector<Item> y_free_items;
        std::vector<Item> z_free_items;
        vector<BinSection> mySections;
        std::unordered_map<std::array<double,3>, array<double,3>, itemPositionHashFunction, itemPositionEqualsFunction> intersectPosDimCaching;


        Bin(string n, double w, double d, double h, double max_w, int estimatedRequiredBins, int numberOfItemsToBePacked) {
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
        };

    vector<Item>& GetFittedItems() { 
        return items; 
    };
    vector<Item>& GetUnfittedItems() { 
        return unfitted_items; 
    };
    double GetActVolumeUtilizationPercentage(){
        return this->act_volume_utilization / this->max_volume * 100;
    };
    double GetActWeightUtilizationPercentage(){
        return this->act_weight_utilization / this->max_weight * 100;
    };
    void IncrementActWeightUtil(double weight) { 
        this->act_weight_utilization += weight; 
    };
    void IncrementActVolumeUtil(double volume) { 
        this->act_volume_utilization += volume; 
    };
    double GetActWeightUtil() { 
        return this->act_weight_utilization; 
    };
    double GetActVolumeUtil() { 
        return this->act_volume_utilization; 
    };
    void UpdateBinSections(const Item& itemToPlace){
        for(auto& binS : this->mySections){
            if(binS.isWithinMyPerimeters(itemToPlace)){
                binS.addItem(itemToPlace);
            };
        };
    };
    void removeFromXFreeItems(const Item& it, int piSysId = -1){
        x_free_items.erase(
                std::remove_if(begin(x_free_items), end(x_free_items), [&](Item& itemInBin) -> bool { 
                        return (it.position[0] == itemInBin.ipwf &&
                            !(it.position[1] >= itemInBin.ipdf && it.ipdf <= itemInBin.position[1]) &&
                            !(it.position[2] >= itemInBin.iphf && it.iphf <= itemInBin.position[2]));
                        }
                    ), end(x_free_items));

        // vector<Item>::iterator xiter = x_free_items.begin();
        // while(xiter != x_free_items.end()) {
        //     if(it.position[0] == xiter->ipwf &&
        //         !(it.position[1] >= xiter->ipdf && it.ipdf <= xiter->position[1]) &&
        //         !(it.position[2] >= xiter->iphf && it.iphf <= xiter->position[2])){
        //         xiter = x_free_items.erase(xiter);
        //     } else{ ++xiter;};
        // };
    };

    void removeFromYFreeItems(const Item& it){
        y_free_items.erase(
                std::remove_if(begin(y_free_items), end(y_free_items), [&](Item& itemInBin) -> bool { 
                    return (it.position[1] == itemInBin.ipdf && 
                            !(it.position[0] >= itemInBin.ipwf && it.ipwf <= itemInBin.position[0]) &&
                            !(it.position[2] >= itemInBin.iphf && it.iphf <= itemInBin.position[2]));
                        }
                    ), end(y_free_items));

        // vector<Item>::iterator yiter = y_free_items.begin();
        // while(yiter != y_free_items.end()) {
        //     if(it.position[1] == yiter->ipdf && 
        //     !(it.position[0] >= yiter->ipwf && it.ipwf <= yiter->position[0]) &&
        //     !(it.position[2] >= yiter->iphf && it.iphf <= yiter->position[2])){
        //         yiter = y_free_items.erase(yiter);
        //     } else{ ++yiter;};
        // };
    };
    void removeFromZFreeItems(const Item& it){
        z_free_items.erase(
                std::remove_if(begin(z_free_items), end(z_free_items), [&](Item& itemInBin) -> bool { 
                    return (itemInBin.iphf == it.position[2] &&
                            itemInBin.position[0] <= it.position[0] && it.position[0] <= itemInBin.ipwf &&
                            itemInBin.position[1] <= it.position[1] && it.position[1] <= itemInBin.ipdf);
                            }
                    ), end(z_free_items));
        // vector<Item>::iterator ziter = z_free_items.begin();
        // while(ziter != z_free_items.end()) {
        //     if(ziter->iphf == it.position[2] &&
        //     ziter->position[0] <= it.position[0] && it.position[0] <= ziter->ipwf &&
        //     ziter->position[1] <= it.position[1] && it.position[1] <= ziter->ipdf){
        //         ziter = z_free_items.erase(ziter);
        //     } else{ ++ziter; }; 
        // };
    };

    void UpdateWithNewFittedItem(const Item& it, const Item& pi, int ax){
        this->items.push_back(it);
        this->UpdateBinSections(it);
        this->IncrementActWeightUtil(it.weight);
        this->IncrementActVolumeUtil(it.volume);

        this->x_free_items.push_back(it);
        this->y_free_items.push_back(it);

        auto hiter = std::upper_bound(this->z_free_items.cbegin(), this->z_free_items.cend(), it, [&](const Item& i1, const Item& i2) { return i1.iphf < i2.iphf; });
        this->z_free_items.insert(hiter, it);

        switch (ax) {
            case AxisHeight: 
                removeFromZFreeItems(it);
                break; 
            default:
                removeFromXFreeItems(it);
                removeFromYFreeItems(it);
                removeFromZFreeItems(it);
        };
    };

    bool PutItem (Item& it){
        bool fit = 0;

        for(int stringCharCounter = 0; stringCharCounter < it.allowed_rotations.size(); stringCharCounter++){
            const std::array<double,3> i_new_dims = it.GetNewItemDimensions(it.allowed_rotations[stringCharCounter] - '0');

            if (   (this->width  < (it.position[0] + i_new_dims[0])) 
                || (this->depth  < (it.position[1] + i_new_dims[1])) 
                || (this->height < (it.position[2] + i_new_dims[2]))){ 
                continue; 
            };
            it.width    = i_new_dims[0];
            it.depth    = i_new_dims[1];
            it.height   = i_new_dims[2];
            it.rotation_type = 0;
            
            std::pair<bool, bool> intersection_result = fit_item(it);
            if(intersection_result.second) { break; };
            if(!intersection_result.first){ continue; } 
            else {
                it.adjusted_rotation_type = it.allowed_rotations[stringCharCounter] - '0';
                return intersection_result.first;
            };
        };
        // if this point is reached then fit==false
        it.width     = it.original_width;
        it.depth     = it.original_depth;
        it.height    = it.original_height;    

        return fit;
    };


    std::pair<bool, bool> fit_item(Item& it){
        bool intersect_fit = 1;
        bool no_rotation_will_help = 0;

        const double ip0 = it.position[0];
        const double ip1 = it.position[1];
        const double ip2 = it.position[2];

        const double ipw = ip0 + it.width;
        const double ipd = ip1 + it.depth;
        const double iph = ip2 + it.height;

        for(auto& binS : this->mySections){
            if(!binS.isWithinMyPerimeters(it)){ continue; };
            const vector<Item>& currentItems = binS.getItems();

            for (int i = currentItems.size(); i--;){     
                if((currentItems[i].ipwf <= ip0 || ipw <= currentItems[i].pw || currentItems[i].ipdf <= ip1 || ipd <= currentItems[i].pd)){
                    continue; 
                };
                
                if(currentItems[i].iphf > ip2 && currentItems[i].ph < iph){

                    no_rotation_will_help = point_in_cuboid(ip0, ip1, ip2, currentItems[i].position, currentItems[i].ipwf, currentItems[i].ipdf, currentItems[i].iphf);
                    const double cache_w = abs(currentItems[i].pw - ip0);
                    const double cache_d = abs(currentItems[i].pd - ip1);
                    const double cache_h = abs(currentItems[i].ph - ip2);

                    const auto& GetIntersectCacheResult = this->intersectPosDimCaching.find(it.position);  
                    if (GetIntersectCacheResult == this->intersectPosDimCaching.end()){
                        this->intersectPosDimCaching[it.position] = {cache_w, cache_d, cache_h};
                    }                    
                    else {
                        this->intersectPosDimCaching[it.position] = {
                            min(GetIntersectCacheResult->second[0], cache_w),
                            min(GetIntersectCacheResult->second[1], cache_d),  
                            min(GetIntersectCacheResult->second[2], cache_h)
                            }; 
                    };

                    intersect_fit = 0;
                    goto functionEnd;
                };
            };
        };
    functionEnd:
        return std::make_pair(intersect_fit, no_rotation_will_help);
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
        std::vector<std::vector<Item>> sortedItemConsKeyVectors;
        bool includeItems;
        bool itemDimensionsAfter;
        bool includeBins;
        double biggestFirst;
        int estimatedTotalRequiredBins;

    public:
        std::vector<Item> items;
        string requestedBinType;
        double requestedBinWidth;
        double requestedBinDepth;
        double requestedBinHeight;
        double requestedBinMaxWeight;
        double requestedBinMaxVolume;

        Packer( string rBinType, double rBinWidth, double rBinDepth, double rBinHeight, double rBinMaxWeight, bool rBiggestFirst, bool rIncludeBins, bool rIncludeItems, bool rItemDimensionsAfter){ 
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
        };

        vector<Item>& GetAllItemsToBePacked() { 
            return this->items; 
        };
        bool GetIncludeBins() { 
            return this->includeBins; 
        };
        std::vector<std::vector<Item>>& GetItemConsKeyVectorsToBePacked() { 
            return this->sortedItemConsKeyVectors; 
        };
        Bin& GetLastBin() { 
            return this->bins.back();
        };
        vector<Bin>& GetPackedBinVector() { 
            return this->bins; 
        };
        void CreateNewBin(string type, double width, double depth, double height, double maxWeight, int estimatedRequiredBins, int numberOfItemsToBePacked){ 
            Bin new_bin(type, width, depth, height, maxWeight, estimatedRequiredBins, numberOfItemsToBePacked);
            this->bins.push_back(new_bin); 
        };
        double GetTotalVolumeUtilizationPercentage(){
            const double maxVolumeUtil = (requestedBinWidth*requestedBinDepth*requestedBinHeight) * this->bins.size();
            double actualVolumeUtil = 0;
            for(auto b : this->bins){
                actualVolumeUtil += b.GetActVolumeUtil();
            };
            return (actualVolumeUtil/maxVolumeUtil * 100);
        };

        void CalculateEstimatedRequiredBins(){
            double summedTotalItemVolume = 0;
            for(auto i : this->items){
                summedTotalItemVolume += i.volume;
            };
            this->estimatedTotalRequiredBins = ceil(summedTotalItemVolume / (this->requestedBinWidth * this-> requestedBinDepth * this->requestedBinHeight));
        };

        double GetTotalWeightUtilizationPercentage(){
            const double maxWeightUtil = this->requestedBinMaxWeight * int(this->bins.size());
            double actualWeightUtil = 0;
            for(auto b : this->bins){
                actualWeightUtil += b.GetActWeightUtil();
            };
            return (actualWeightUtil/maxWeightUtil * 100);
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

            if(regex_match(to_string(item.adjusted_rotation_type), validAdjustedRotationType)){
                JsonItem["rotationType"] = item.adjusted_rotation_type;
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
                mappedBin["fittedItems"] = Json::arrayValue;
                for(auto& it : bin.GetFittedItems()){
                    mappedBin["fittedItems"].append(this->MapItemToJSON(it));
                };
            };
            return mappedBin;
        };

        void UpdateItemThatHasBeenFit(Item& i){
            i.SetRotationTypeDesc();
            i.SetItemDimensionInfo();
        };

        void sortItemsByConsKey(){
            std::sort(this->items.begin(), this->items.end(), [&](const Item& i1, const Item& i2){return i1.item_cons_key > i2.item_cons_key;});
            for(const auto& i : this->items){
                if(this->sortedItemConsKeyVectors.empty()){
                    this->sortedItemConsKeyVectors.push_back(vector<Item> {i});
                    continue;
                };
                if(i.item_cons_key == this->sortedItemConsKeyVectors.back().back().item_cons_key){
                    this->sortedItemConsKeyVectors.back().push_back(i);
                } else{
                    this->sortedItemConsKeyVectors.push_back(vector<Item> {i});
                };
            };
        };

        void pack_to_bin(Bin& bin, Item& item_to_fit){
            bool fitted = 0;
            vector<Item> items_in_bin;
            for(const auto axis : AllAxis){
                switch (axis){
                    case AxisWidth:
                        items_in_bin = bin.x_free_items;
                        break;
                    case AxisDepth:
                        items_in_bin = bin.y_free_items;
                        break;
                    case AxisHeight:
                        items_in_bin = bin.z_free_items;
                        break;
                };

                for(const auto& item_in_bin : items_in_bin){
                    item_to_fit.position = item_in_bin.position;
                    switch (axis){
                        case AxisWidth:
                            item_to_fit.position[0] += item_in_bin.width;
                            break;
                        case AxisDepth:
                            item_to_fit.position[1] += item_in_bin.depth;
                            break;
                        case AxisHeight:
                            item_to_fit.position[2] += item_in_bin.height;
                            break;
                    };
                    const auto& GetIntersectCachePBResult = bin.intersectPosDimCaching.find(item_to_fit.position);
                    if (GetIntersectCachePBResult != bin.intersectPosDimCaching.end()){
                        if( item_to_fit.width >= GetIntersectCachePBResult->second[0] 
                            && item_to_fit.depth >= GetIntersectCachePBResult->second[1] 
                            && item_to_fit.height >= GetIntersectCachePBResult->second[2] ){
                            continue;
                        };
                    };

                    if(bin.PutItem(item_to_fit)){
                        fitted = 1;
                        this->UpdateItemThatHasBeenFit(item_to_fit);
                        bin.UpdateWithNewFittedItem(item_to_fit, item_in_bin, axis);
                        break;
                    
                    };
                };
                if(fitted){ break; };
            };
            if(!fitted){ bin.GetUnfittedItems().push_back(item_to_fit);};
        };

        void startPacking(std::vector<Item> itemsToBePacked){
            if(itemsToBePacked.empty()){ return; };
            
            this->CreateNewBin(this->requestedBinType + "-" + to_string(this->bins.size()+1),
                                this->requestedBinWidth,
                                this->requestedBinDepth,
                                this->requestedBinHeight,
                                this->requestedBinMaxWeight,
                                this->estimatedTotalRequiredBins,
                                itemsToBePacked.size());
            if(this->biggestFirst){
                std::sort(itemsToBePacked.begin(), itemsToBePacked.end(), sortItemsOnVolume);
            };

            for(auto& item_to_pack : itemsToBePacked){
                if(this->GetLastBin().GetFittedItems().empty()){ 
                    item_to_pack.position = START_POSITION;
                    if(this->GetLastBin().PutItem(item_to_pack)){ 
                        this->UpdateItemThatHasBeenFit(item_to_pack);
                        this->GetLastBin().UpdateWithNewFittedItem(item_to_pack, item_to_pack, 9);
                        continue;
                    };
                };
                if(!this->GetLastBin().GetUnfittedItems().empty()){
                    if(this->GetLastBin().GetUnfittedItems().back().width == item_to_pack.width && this->GetLastBin().GetUnfittedItems().back().height == item_to_pack.height && this->GetLastBin().GetUnfittedItems().back().depth == item_to_pack.depth){
                            this->GetLastBin().GetUnfittedItems().push_back(item_to_pack);
                            continue;
                    };
                };
                if((this->GetLastBin().GetActVolumeUtil() + item_to_pack.volume) > this->GetLastBin().max_volume || (this->GetLastBin().GetActWeightUtil() + item_to_pack.weight) > this->GetLastBin().max_weight){
                        this->GetLastBin().GetUnfittedItems().push_back(item_to_pack);
                        continue;
                };
                this->pack_to_bin(this->GetLastBin(), item_to_pack);
            };
            
            if(itemsToBePacked.size() == this->GetLastBin().GetUnfittedItems().size()){ 
                if(this->GetLastBin().GetFittedItems().empty()){
                    this->GetPackedBinVector().pop_back();
                };
                return;
            };
            startPacking(this->GetLastBin().GetUnfittedItems());      
        };
};


/* 
    ----------------------------------------------------
                        INPUT READER
    ----------------------------------------------------
        Reads JSON input, contains ITEMS and BIN        
*/
// extern "C" {
// const char * packToBinAlgorhitm(char* incomingJson, bool biggestFirst = 1,
//                                     bool includeBins = 1, bool includeItems = 1, bool itemDimensionsAfter = 0,
//                                    int jsonPrecision = 5) {

int main() {
    auto start = chrono::high_resolution_clock::now();
    bool biggestFirst           = 1;
    bool includeBins            = 1;
    bool includeItems           = 1;
    bool itemDimensionsAfter    = 1;
    int jsonPrecision           = 5;

    ifstream incomingJson ("./cpp_vs_python_5000_items.json");
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
                            itemDimensionsAfter
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

    PackingProcessor.sortItemsByConsKey();
    PackingProcessor.CalculateEstimatedRequiredBins();

    for(auto& sortedItemConsKeyVector : PackingProcessor.GetItemConsKeyVectorsToBePacked()){
        PackingProcessor.startPacking(sortedItemConsKeyVector);
    };

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << duration.count() << endl;

    /* OUTGOING JSON PARSING */
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder.settings_["precision"] = jsonPrecision;
    Json::Value outboundRoot; 

    outboundRoot["requiredNrOfBins"] = int(PackingProcessor.GetPackedBinVector().size());
    outboundRoot["totalVolumeUtil"] = PackingProcessor.GetTotalVolumeUtilizationPercentage();
    outboundRoot["totalWeightUtil"] = PackingProcessor.GetTotalWeightUtilizationPercentage();
    outboundRoot["unfittedItems"] = Json::arrayValue;

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
        outboundRoot["packedBins"] = Json::arrayValue;

        for(auto& bi : PackingProcessor.GetPackedBinVector()){
            outboundRoot["packedBins"].append(PackingProcessor.MapBinToJSON(bi));
            cout << bi.name << " size: " << bi.GetFittedItems().size() << " MaxWeight: " << PackingProcessor.requestedBinMaxWeight <<  "\n";
            for(auto& bs : bi.mySections){
                bs.printMe();
            };
        };
    };
    // cout << intersectCounter << " -- " << xaxis << " " << yaxis << " " << zaxis << "\n";
    cout << PackingProcessor.GetLastBin().x_free_items.size() << " " << PackingProcessor.GetLastBin().y_free_items.size() << " " << PackingProcessor.GetLastBin().z_free_items.size() << "\n";

    // cout << output << "\n";
    // ofstream myfile;
    // myfile.open ("ex1.txt");
    // string output = Json::writeString(builder, outboundRoot);
    // myfile << output;
    // myfile.close();

    // return strdup(Json::writeString(builder, outboundRoot).c_str());
    return 0;
// };

};

extern "C" {
const void packToBinAlgorhitmFreeMemory(char *outputPtr) {
    free(outputPtr);
    };
};