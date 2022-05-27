#include "constants.h"

class Partitioner {
    private:
        int partitionLevel_;
        int estimatedRequiredBins_;
        int numberOfItemsToBePacked_;
        std::vector<BinSection> binSections_;
        std::array<double,3> masterBinStartingPosition_;
        std::array<double,3> masterBinMaxDimensions_;

    public:
        Partitioner(int reqEstimatedTotalBinsToBePacked, int reqNumberOfItemsToBePacked, std::array<double,3> startPos, std::array<double,3> maxPos) {
            masterBinStartingPosition_ = startPos;
            masterBinMaxDimensions_ = maxPos;
            estimatedRequiredBins_ = reqEstimatedTotalBinsToBePacked;
            numberOfItemsToBePacked_ = reqNumberOfItemsToBePacked;

            BinSection singleSection(binSections_.size(), masterBinStartingPosition_, masterBinMaxDimensions_);
            binSections_.push_back(singleSection);

            calculatePartitionLevel();
            generateBinSections();
        };
        std::vector<BinSection>& getBinSections(){
            return binSections_;
        };
        void calculatePartitionLevel(){
            //the double is the desired number of items per bin section
            partitionLevel_ = ceil(sqrt(((numberOfItemsToBePacked_/estimatedRequiredBins_)/750.0)));
        };
       void singlePartition(std::array<double,3> startPos, std::array<double,3> maxPos, int axis){
            double partitionBorder = maxPos[axis] - abs(startPos[axis]-maxPos[axis]) / 2;

            std::array<double,3> part1 = maxPos;
            part1[axis] = partitionBorder;
            BinSection newSection1(binSections_.size(), startPos, part1);
            binSections_.push_back(newSection1);

            std::array<double,3> part2 = startPos;
            part2[axis] = partitionBorder;
            BinSection newSection2(binSections_.size(), part2, maxPos);
            binSections_.push_back(newSection2);
        };
        void partitionEachBinSection(int axis){
            std::vector<BinSection> cbins = binSections_;
            binSections_.clear();
            for(auto& binS : cbins){
                this->singlePartition(binS.getBinSectionStart(),binS.getBinSectionEnd(), axis);
            };
        };
        void generateBinSections(){
            if(partitionLevel_ == 1) { return; };

            int PartitionAxis = constants::AxisWidth;
            for(int i=0; i < partitionLevel_; i++){
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