/*
    GRAVITY.
        Handles logic related to gravity.

    - Boxes being stacked in the air can happen since depth stacking is checked on all items before height stacking.
    - Boxes resting partly on a box.

    Prevent item floatation by returning an invalid position response when no item is under the position being evaluated.
*/

class Gravity{
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