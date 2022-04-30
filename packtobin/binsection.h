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
            std::cout << "ID: " <<this->id << " size: " << this->items.size() << " start: ";
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