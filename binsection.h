class BinSection {
    private:
        int id_;
        std::vector<Item> items_;
        std::array<double,3> sectionStart_;
        std::array<double,3> sectionEnd_;

    public:        
        BinSection(int myId, std::array<double,3> startPos, std::array<double,3> maxPos){
            id_ = myId;
            sectionStart_ = startPos;
            sectionEnd_ = maxPos;
        };
        std::array<double,3> getBinSectionStart(){
            return sectionStart_;
        };
        std::array<double,3> getBinSectionEnd(){
            return sectionEnd_;
        };
        void addItem(const Item& i){
            items_.push_back(i);
        };
        const std::vector<Item>& getItems() {
            return items_;
        };
        bool isWithinMyR3Perimeters(const Item& itemToCheck){
            return !(sectionEnd_[0]    <= itemToCheck.position_[0] 
                    || (itemToCheck.position_[0]+itemToCheck.width_)     <= sectionStart_[0]
                    || sectionEnd_[1]  <= itemToCheck.position_[1] 
                    || (itemToCheck.position_[1]+itemToCheck.depth_)     <= sectionStart_[1])
                    && (sectionEnd_[2] > itemToCheck.position_[2] && sectionStart_[2] < (itemToCheck.position_[2]+itemToCheck.height_));
        };
        bool isWithinMyR2XYPerimeters(const Item& itemToCheck){
            return !(sectionEnd_[0]    <= itemToCheck.position_[0] 
                    || (itemToCheck.position_[0]+itemToCheck.width_)     <= sectionStart_[0]
                    || sectionEnd_[1]  <= itemToCheck.position_[1] 
                    || (itemToCheck.position_[1]+itemToCheck.depth_)     <= sectionStart_[1]);
        };
        void printMe(){
            std::cout << "ID: " << id_ << " size: " << items_.size() << " start: ";
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