#include <iostream>
#include <fstream>

#include <unordered_map>
#include <vector>

#include <jsoncpp/json/json.h>
#include <chrono>
#include <cmath>
#include <regex>


#include "itemregister.h"
#include "item.h"
#include "gravity.h"
#include "binsection.h"
#include "partitioner.h"
#include "bin.h"
#include "packer.h"
#include "constants.h"



/* 
    ----------------------------------------------------
                        PACK TO BIN
    ----------------------------------------------------
        Reads JSON input, contains ITEMS and BIN
        
        Returns JSON         
*/


// extern "C" {
// const char * packToBinAlgorhitm(char* incomingJson, bool biggestFirst=1,
//                                 bool includeBins=1, bool includeItems=1, 
//                                 bool itemDimensionsAfter=0, int jsonPrecision=5,
//                                 double gravityStrength=0.0) {

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    bool biggestFirst           = 1;
    bool includeBins            = 1;
    bool includeItems           = 1;
    bool itemDimensionsAfter    = 0;
    int jsonPrecision           = 5;
    double gravityStrength      = 0.0;

    std::ifstream incomingJson ("/home/moschd/Desktop/cpp/testfiles/testjson copy.json");
    Json::Reader reader;
    Json::Value inboundRoot;
    reader.parse(incomingJson, inboundRoot);
    Json::Value ijb = inboundRoot["bin"];
    Json::Value iji = inboundRoot["items"];
    
    //init Packer
    Packer PackingProcessor(ijb["type"].asString(),
                            ijb["width"].asDouble(),
                            ijb["depth"].asDouble(),
                            ijb["height"].asDouble(),
                            ijb["maxWeight"].asDouble(),
                            biggestFirst,
                            includeBins,
                            includeItems,
                            itemDimensionsAfter,
                            gravityStrength
                            );

    //create item objects
    itemRegister itemReg;

    for (int x=iji.size(); x--;) {
        // Item i = ;
        PackingProcessor.items.push_back(Item(x,
                                            iji[x]["id"].asString(),
                                            iji[x]["width"].asDouble(),
                                            iji[x]["depth"].asDouble(),
                                            iji[x]["height"].asDouble(),
                                            iji[x]["weight"].asDouble(),
                                            iji[x]["itemConsKey"].asString(),
                                            iji[x]["allowedRotations"].asString()));
        // itemReg.fillRegister(Item(x,
        //                                     iji[x]["id"].asString(),
        //                                     iji[x]["width"].asDouble(),
        //                                     iji[x]["depth"].asDouble(),
        //                                     iji[x]["height"].asDouble(),
        //                                     iji[x]["weight"].asDouble(),
        //                                     iji[x]["itemConsKey"].asString(),
        //                                     iji[x]["allowedRotations"].asString()));
    };


    PackingProcessor.CreateFinalSortedItemConsKeyVectorsAndEstimateBins();
    // for(auto nr : itemReg.completeItemMap){
    //     std::cout << "new map " << nr.first << " " << nr.second.id << " " <<nr.second.width << "\n";
    // }
    for(auto& sortedItemConsKeyVector : PackingProcessor.GetItemConsKeyVectorsToBePacked()){
        PackingProcessor.startPacking(sortedItemConsKeyVector);
    };

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << std::endl;


    /* OUTGOING JSON BUILDER */
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder.settings_["precision"] = jsonPrecision;
    Json::Value outboundRoot; 

    outboundRoot["requiredNrOfBins"]= int(PackingProcessor.GetPackedBinVector().size());
    outboundRoot["totalVolumeUtil"] = PackingProcessor.GetTotalVolumeUtilizationPercentage();
    outboundRoot["totalWeightUtil"] = PackingProcessor.GetTotalWeightUtilizationPercentage();
    outboundRoot["unfittedItems"]   = Json::arrayValue;

    if(!PackingProcessor.GetLastBin().GetUnfittedItems().empty()){
        for(auto& it : PackingProcessor.GetLastBin().GetUnfittedItems()){
            outboundRoot["unfittedItems"].append(PackingProcessor.MapItemToJSON(it));
        };
    };

    if(PackingProcessor.GetIncludeBins()){
        outboundRoot["binDetails"]["maxWidth"]  = PackingProcessor.requestedBinWidth;
        outboundRoot["binDetails"]["maxDepth"]  = PackingProcessor.requestedBinDepth;
        outboundRoot["binDetails"]["maxHeight"] = PackingProcessor.requestedBinHeight;
        outboundRoot["binDetails"]["maxWeight"] = PackingProcessor.requestedBinMaxWeight;
        outboundRoot["binDetails"]["maxVolume"] = PackingProcessor.requestedBinMaxVolume;

        for(auto& bi : PackingProcessor.GetPackedBinVector()){
            outboundRoot["packedBins"].append(PackingProcessor.MapBinToJSON(bi));
            std::cout << bi.name << " size: " << bi.GetFittedItems().size() << " MaxWeight: " << PackingProcessor.requestedBinMaxWeight <<  "\n";
            std::cout << "-- Find Single Point {0.58,0.49,0.64} --\n" << searchTree(bi.kdTree,{0.58,0.49,0.64}) << "\n";
            std::cout << "-- Return Intersection Candidates --" << "\n";
            for(auto r : searchItemTreeAndReturnIntersectionCandidates(bi.kdTree,{0.0,0.0,0.0})){
                std::cout << r << " ";
            };
            std::cout << "\n-- --\n";
            for(auto& bs : bi.mySections){
                bs.printMe();
            };
            std::cout << "-- #### --\n";
        };
    };
    std::cout << itemReg.completeItemMap.size();
    // std::cout << output << "\n";
    // std::ofstream myfile;
    // myfile.open ("ex1.txt");
    // std::string output = Json::writeString(builder, outboundRoot);
    // myfile << output;
    // myfile.close();

   /*double vm, rss;
   process_mem_usage(vm, rss);
   std::cout << "VM: " << vm << "; RSS: " << rss << "\n";*/
    // return strdup(Json::writeString(builder, outboundRoot).c_str());
    return 0;
// };

};


extern "C" {
const void packToBinAlgorhitmFreeMemory(char *outputPtr) {
    free(outputPtr);
    };
};












/*
#include <iostream>
#include <fstream>
#include <unistd.h>

void process_mem_usage(double& vm_usage, double& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}*/