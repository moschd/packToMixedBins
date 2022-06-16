#include <jsoncpp/json/json.h>
#include "packer.h"
#include "item.h"
#include "bin.h"
#include "itemregister.h"

class jsonResponseBuilder{
    private:
        int precision_;
        std::string indentation_;
        bool includeItems_;
        bool includeBins_;
        bool itemDimensionsAfter_;
        Json::StreamWriterBuilder builder_;
        Json::Value outboundRoot_;

        void configureBuilder(){
            builder_["indentation"] = indentation_;
            builder_.settings_["precision"] = precision_;
        };


        Json::Value itemToJson(const Item& item){
            Json::Value JsonItem;
            JsonItem[constants::json::item::ID] = item.id_;
            JsonItem[constants::json::item::WIDTH]   = (itemDimensionsAfter_ ? item.width_   : item.original_width_);
            JsonItem[constants::json::item::DEPTH]   = (itemDimensionsAfter_ ? item.depth_   : item.original_depth_);
            JsonItem[constants::json::item::HEIGHT]  = (itemDimensionsAfter_ ? item.height_  : item.original_height_);
            JsonItem[constants::json::item::WEIGHT] = item.weight_;
            JsonItem[constants::json::item::VOLUME] = item.volume_;

            if(item.itemConsolidationKey_.size()){
                JsonItem[constants::json::item::ITEM_CONS_KEY] = item.itemConsolidationKey_;
            };

            JsonItem[constants::json::item::ALLOWED_ROTATIONS] = item.allowedRotations_;
            JsonItem[constants::json::item::X_COORDINATE] = item.position_[0];
            JsonItem[constants::json::item::Y_COORDINATE] = item.position_[1];
            JsonItem[constants::json::item::Z_COORDINATE] = item.position_[2];

            if(0 <= item.rotationType_ && item.rotationType_ < 6){
                JsonItem[constants::json::item::ROTATION_TYPE] = item.rotationType_;
            };

            if(item.rotationTypeDescription_.size()){
                JsonItem[constants::json::item::ROTATION_TYPE_DESCRIPTION] = item.rotationTypeDescription_;
            };

            return JsonItem;
        };


        Json::Value binToJson(Bin& bin){
            Json::Value mappedBin;
            mappedBin[constants::json::bin::BIN_NAME]            = bin.name;
            mappedBin[constants::json::bin::NR_OF_ITEMS]          = int(bin.GetFittedItems().size());
            mappedBin[constants::json::bin::ACTUAL_VOLUME]       = bin.GetActVolumeUtil();
            mappedBin[constants::json::bin::ACTUAL_VOLUME_UTIL]   = bin.GetActVolumeUtilizationPercentage();
            mappedBin[constants::json::bin::ACTUAL_WEIGHT]       = bin.GetActWeightUtil();
            mappedBin[constants::json::bin::ACTUAL_WEIGHT_UTIL]   = bin.GetActWeightUtilizationPercentage();
            return mappedBin;
        };


        void exceptionJson(int exceptionType){
            switch(exceptionType){
                case 10:
                    outboundRoot_[constants::json::outbound::EXCEPTION] = "Every single item exceeds the bin capacity. None of the items could be packed.";
                    break;
            };
        }



    public:
        
        Json::StreamWriterBuilder& getBuilder(){ return builder_; };
        
        Json::Value& getMessage(){ return outboundRoot_; };
        
        jsonResponseBuilder(int rPrecision, bool rIncludeBins, bool rIncludeItems, bool rItemDimensionsAfter){
            indentation_ = "";
            precision_ = rPrecision;
            includeBins_ = rIncludeBins;
            includeItems_ = rIncludeItems;
            itemDimensionsAfter_ = rItemDimensionsAfter;

            configureBuilder();
        };


        /**
         * @brief Generates the outgoing JSON.
         *  
         * It takes a packed packing processor as input and converts its content to the outgoing json.
         * Generate a exception json incase none of the provided items are packed into a bin.
         * 
         * @param PackingProcessor 
         */
        void generate(Packer PackingProcessor){

            if(!PackingProcessor.GetPackedBinVector().size()){ 
                exceptionJson(10);
                return; 
            };

            // Header information
            outboundRoot_[constants::json::outbound::header::REQUIRED_NR_OF_BINS]= int(PackingProcessor.GetPackedBinVector().size());
            outboundRoot_[constants::json::outbound::header::TOTAL_VOLUME_UTIL] = PackingProcessor.GetTotalVolumeUtilizationPercentage();
            outboundRoot_[constants::json::outbound::header::TOTAL_WEIGHT_UTIL] = PackingProcessor.GetTotalWeightUtilizationPercentage();


            // Check for items that did not get packed and include them in the header.
            if(!PackingProcessor.GetLastBin().GetUnfittedItems().empty()){
                outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS]   = Json::arrayValue;
                for(auto& it : PackingProcessor.GetLastBin().GetUnfittedItems()){
                    outboundRoot_[constants::json::outbound::header::UNFITTED_ITEMS].append(itemToJson(PackingProcessor.masterItemRegister_->getItem(it)));
                };

            };

            if(!includeBins_) { return; };

            // Only relevant if bins need to be included in the response.
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::TYPE]       = PackingProcessor.requestedBinType_;
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::MAX_WIDTH]  = PackingProcessor.requestedBinWidth_;
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::MAX_DEPTH]  = PackingProcessor.requestedBinDepth_;
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::MAX_HEIGHT] = PackingProcessor.requestedBinHeight_;
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::MAX_WEIGHT] = PackingProcessor.requestedBinMaxWeight_;
            outboundRoot_[constants::json::outbound::BIN_DETAILS][constants::json::outbound::bindetails::MAX_VOLUME] = PackingProcessor.requestedBinMaxVolume_;

            for(auto& bi : PackingProcessor.GetPackedBinVector()){
                Json::Value mappedBin = binToJson(bi);

                if(includeItems_){
                    for(auto& it : bi.GetFittedItems()){
                        mappedBin[constants::json::bin::FITTED_ITEMS].append(itemToJson(PackingProcessor.masterItemRegister_->getItem(it)));
                    };
                };
                outboundRoot_[constants::json::outbound::PACKED_BINS].append(mappedBin);
            };
        };
};