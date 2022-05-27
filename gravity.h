/*
    GRAVITY.
        Handles logic related to gravity.

    - Boxes being stacked in the air can happen since depth stacking is checked on all items before height stacking.
    - Boxes resting partly on a box.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

class Gravity{
    public:
        bool gravityEnabled_;
        double gravityStrengthPercentage_;
    
        Gravity(double rGravityStrengthPercentage){
            gravityStrengthPercentage_ = rGravityStrengthPercentage - 0.1;
            gravityEnabled_ = (rGravityStrengthPercentage > 0.0 ? 0 : 1);
        };
        Gravity(){
            gravityStrengthPercentage_ = 0.0;
        }
        /*
        double R2XYItemSurfaceAreaCoveragePercentage(const double floor, const std::vector<Item>& itemsInSpace, const Item& itemToBeChecked){
            if(floor == itemToBeChecked.position_[2]) { return 100.0; };
            
            double totalCoveredSurfaceAreaPercentage = 0.0;
            for(auto& itemInSpace : itemsInSpace){
                if(itemInSpace.iphf_ != itemToBeChecked.position_[2]) { continue; };
                if( !(itemInSpace.ipwf_ <= itemToBeChecked.position_[0] || 
                    (itemToBeChecked.position_[0] + itemToBeChecked.width_) <= itemInSpace.position_[0] || 
                    itemInSpace.ipdf_ <= itemToBeChecked.position_[1] || 
                    (itemToBeChecked.position_[1] + itemToBeChecked.depth_) <= itemInSpace.position_[1])) {

                        //calculate the surface area that this box is covering on the box to be placed
                        totalCoveredSurfaceAreaPercentage +=    std::max(0.0, std::min(itemInSpace.ipwf_,        (itemToBeChecked.position_[0] + itemToBeChecked.width_)) 
                                                                            - std::max(itemInSpace.position_[0], itemToBeChecked.position_[0])) *
                                                                std::max(0.0, std::min(itemInSpace.ipdf_,        (itemToBeChecked.position_[1] + itemToBeChecked.depth_)) 
                                                                            - std::max(itemInSpace.position_[1], itemToBeChecked.position_[1])) 
                                                                / (itemToBeChecked.width_*itemToBeChecked.depth_) * 100; 

                        if(totalCoveredSurfaceAreaPercentage > gravityStrengthPercentage_) { break; };
                    };
                };
            return totalCoveredSurfaceAreaPercentage;        
        };

        inline double R2XYItemSurfaceAreaCoveragePercentage(const double floor, const Item& itemInSpace, const Item& itemToBeChecked){
            if(floor == itemToBeChecked.position_[2]) { return 100.0; };

            return  std::max(0.0, std::min(itemInSpace.ipwf_, (itemToBeChecked.position_[0] + itemToBeChecked.width_)) 
                                - std::max(itemInSpace.position_[0], itemToBeChecked.position_[0])) *
                    std::max(0.0, std::min(itemInSpace.ipdf_, (itemToBeChecked.position_[1] + itemToBeChecked.depth_)) 
                                - std::max(itemInSpace.position_[1], itemToBeChecked.position_[1])) 
                    / (itemToBeChecked.width_*itemToBeChecked.depth_) * 100;        
        };
        */
};