#ifndef PACKER_H
#define PACKER_H
class Packer {
    private:
        std::vector<Bin> bins_;
        std::vector<std::vector<int>> FinalSortedItemConsKeyVectors_;
        int estimatedTotalRequiredBins_;

    public:
        std::string requestedBinType_;
        double requestedBinWidth_;
        double requestedBinDepth_;
        double requestedBinHeight_;
        double requestedBinMaxWeight_;
        double requestedBinMaxVolume_;
        double requestedGravityStrength_;
        itemRegister* masterItemRegister_;


        Packer(std::string rBinType, double rBinWidth, double rBinDepth, double rBinHeight, double rBinMaxWeight, 
                double rGravityStrength, itemRegister& rItemRegister){ 
            requestedBinType_        = rBinType;
            requestedBinWidth_       = rBinWidth;
            requestedBinDepth_       = rBinDepth;
            requestedBinHeight_      = rBinHeight;
            requestedBinMaxWeight_   = rBinMaxWeight;
            requestedBinMaxVolume_   = (rBinWidth * rBinHeight * rBinDepth);
            requestedGravityStrength_= rGravityStrength;
            masterItemRegister_      = &rItemRegister;
        };

        std::vector<std::vector<int>>& GetItemConsKeyVectorsToBePacked() { 
            return FinalSortedItemConsKeyVectors_; 
        };
        Bin& GetLastBin() { 
            return bins_.back();
        };
        std::vector<Bin>& GetPackedBinVector() { 
            return bins_; 
        };

        double GetTotalVolumeUtilizationPercentage(){
            double actualVolumeUtil = 0;
            for(auto& b : bins_){
                actualVolumeUtil += b.GetActVolumeUtil();
            };
            return std::max(0.0, actualVolumeUtil / (requestedBinWidth_*requestedBinDepth_*requestedBinHeight_ * bins_.size()) * 100);
        };

        double GetTotalWeightUtilizationPercentage(){
            double actualWeightUtil = 0;
            for(auto& b : bins_){
                actualWeightUtil += b.GetActWeightUtil();
            };
            return std::max(0.0, actualWeightUtil / (requestedBinMaxWeight_ * bins_.size()) * 100);
        };


        /**
         * @brief Create volume sorted item input vectors split by consolidation key.
         * 
         * Sorts items based on item consolidation key and volume and created separate stacking input std::vectors.
         * 
         */
        void CreateFinalSortedItemConsKeyVectors(){
            std::sort(masterItemRegister_->getCompleteItemVector().begin(), masterItemRegister_->getCompleteItemVector().end(), consKeyAndVolumeSorter());
            
            FinalSortedItemConsKeyVectors_.push_back(std::vector<int> {masterItemRegister_->getCompleteItemVector()[0].transientSysId_} );

            for(int idx = 1; idx < masterItemRegister_->getCompleteItemVector().size(); idx++){

                if( masterItemRegister_->getCompleteItemVector()[idx].itemConsolidationKey_ == 
                    masterItemRegister_->getItem(FinalSortedItemConsKeyVectors_.back().back()).itemConsolidationKey_){
                    FinalSortedItemConsKeyVectors_.back().push_back(masterItemRegister_->getCompleteItemVector()[idx].transientSysId_);
                } else {
                    FinalSortedItemConsKeyVectors_.push_back(std::vector<int> {masterItemRegister_->getCompleteItemVector()[idx].transientSysId_});
                };
            };
        };



        /**
         * @brief Start to add item(s) into a bin.
         * 
         * This method iterates over the item vector and tries to place each item into the bin. If a bin is full it
         * creates a new bin and the process starts over, now the input is the previous bin' unfitted items.
         * 
         * @param itemsToBePacked 
         */
        void startPacking(std::vector<int> itemsToBePacked){
            if(itemsToBePacked.empty()){ return; };
            
            Bin new_bin(requestedBinType_ + "-" + std::to_string(bins_.size()+1),
                        requestedBinWidth_,
                        requestedBinDepth_,
                        requestedBinHeight_,
                        requestedBinMaxWeight_,
                        itemsToBePacked.size(),
                        requestedGravityStrength_,
                        masterItemRegister_);

            bins_.push_back(new_bin);

            for(auto& item_to_pack : itemsToBePacked){
                Item* itp = &masterItemRegister_->getItem(item_to_pack);
        
                /* Check if item would exceed weight or volume limit. */
                if((bins_.back().GetActVolumeUtil() + itp->volume_) > bins_.back().maxVolume_ ||
                    (bins_.back().GetActWeightUtil() + itp->weight_) > bins_.back().maxWeight_){
                    bins_.back().GetUnfittedItems().push_back(itp->transientSysId_);
                    continue;
                };

                /* Check if item would be the first item in the bin, if so take shortcut. */
                if(bins_.back().GetFittedItems().empty()){ 
                    if(bins_.back().PlaceItemInBin(item_to_pack)){ 
                        bins_.back().UpdateWithNewFittedItem(item_to_pack, 0);
                        continue;
                    };
                };

                /* Check if item is the same as previous unfitted item. */
                if(!bins_.back().GetUnfittedItems().empty() && 
                    masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).width_ == itp->width_ && 
                    masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).height_ == itp->height_ && 
                    masterItemRegister_->getItem(bins_.back().GetUnfittedItems().back()).depth_ == itp->depth_){
                    bins_.back().GetUnfittedItems().push_back(itp->transientSysId_);
                    continue;
                };

                bins_.back().FindItemPosition(item_to_pack);

            };

            // Delete the created bin when if it contains no items.
            if(itemsToBePacked.size() == bins_.back().GetUnfittedItems().size()){ 
                GetPackedBinVector().pop_back();
                return;
            };

            startPacking(bins_.back().GetUnfittedItems());      
        };
};

#endif