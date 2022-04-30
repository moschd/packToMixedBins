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

            if(0 <= item.rotation_type < 6){
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