#ifndef ITEM_H
#define ITEM_H
#include "constants.h"


class Item {
    public:
        int transientSysId;
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
            transientSysId = systemId;
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
            position = constants::START_POSITION;
        };

    // bool operator== (const Item& other) const {
    //     return transientSysId == other.transientSysId ? 1 : 0;
    // };

    inline void SetNewItemDimensions(const int reqRotationType) {
        switch (reqRotationType){
            case constants::RT_WDH: // keep {width, depth, height};
                this->width = this->original_width;
                this->depth = this->original_depth;
                this->height = this->original_height;
                break;
            case constants::RT_DWH: // {depth, width, height};
                this->width = this->original_depth;
                this->depth = this->original_width;
                this->height = this->original_height;
                break;
            case constants::RT_HDW: // {height, depth, width};
                this->width = this->original_height;
                this->depth = this->original_depth;
                this->height = this->original_width;
                break;
            case constants::RT_DHW: // {depth, height, width};
                this->width = this->original_depth;
                this->depth = this->original_height;
                this->height = this->original_width;
                break;
            case constants::RT_HWD: // {height, width, depth};
                this->width = this->original_height;
                this->depth = this->original_width;
                this->height = this->original_depth;
                break;
            case constants::RT_WHD: // {width, height, depth};
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
            case constants::RT_WDH:
                this->rotation_type_description = "No box rotation";
                break;
            case constants::RT_DWH:
                this->rotation_type_description = "Rotate the box around the z-axis by 90°";
                break;
            case constants::RT_HDW:
                this->rotation_type_description = "Rotate the box around the x-axis by 90°";
                break;
            case constants::RT_DHW:
                this->rotation_type_description = "Rotate the box around the x-axis by 90° and then around the z-axis by 90°";
                break;
            case constants::RT_WHD:
                this->rotation_type_description = "Rotate the box around the y-axis by 90°";
                break;
            case constants::RT_HWD:
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

#endif