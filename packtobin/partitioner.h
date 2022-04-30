#include "constants.h"

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

            int PartitionAxis = constants::AxisWidth;
            for(int i=0; i < this->partitionLevel; i++){
                this->partitionEachBinSection(PartitionAxis);
                switch(PartitionAxis){
                    case constants::AxisWidth:
                        PartitionAxis = constants::AxisDepth;
                        break;
                    case constants::AxisDepth:
                        PartitionAxis = constants::AxisHeight;
                        break;
                    case constants::AxisHeight:
                        PartitionAxis = constants::AxisWidth;
                        break;
                };
            };
        };
};