#include <iostream>
#include <fstream>

#include <unordered_map>
#include <vector>

#include <jsoncpp/json/json.h>
#include <chrono>
#include <cmath>
#include <regex>

#include "constants.h"
#include "item.h"
#include "itemregister.h"
#include "gravity.h"
#include "bin.h"
#include "packer.h"
#include "outgoingJsonBuilder.h"

/* 
    ----------------------------------------------------
                        PACK TO BIN
    ----------------------------------------------------    
    Input:
        JSON
    Output:
        JSON  
*/




extern "C" {
const char * packToBinAlgorhitm(char* incomingJson, bool includeBins=1, bool includeItems=1, 
                bool itemDimensionsAfter=0, int jsonPrecision=5, double gravityStrength=0.0) {


// int main() {
//     auto start = std::chrono::high_resolution_clock::now();
//     bool includeBins            = 1;
//     bool includeItems           = 1;
//     bool itemDimensionsAfter    = 0;
//     int jsonPrecision           = 5;
//     double gravityStrength      = 0.0;

//     std::ifstream incomingJson ("/home/moschd/packToBin/testfiles/testjson copy.json");
    // std::ifstream incomingJson ("/home/moschd/packToBin/testfiles/cpp_vs_python_2500_items.json");
    // std::ifstream incomingJson ("/home/moschd/packToBin/testfiles/cpp_vs_python_5000_items.json");
    Json::Reader reader;
    Json::Value inboundRoot;
    reader.parse(incomingJson, inboundRoot);
    Json::Value ijb = inboundRoot[constants::json::inbound::BIN];
    Json::Value iji = inboundRoot[constants::json::inbound::ITEMS];
    
    // Create item register
    itemRegister masterItemRegister;

    // Init Packer
    Packer PackingProcessor(ijb[constants::json::packer::TYPE].asString(),
                            ijb[constants::json::packer::WIDTH].asDouble(),
                            ijb[constants::json::packer::DEPTH].asDouble(),
                            ijb[constants::json::packer::HEIGHT].asDouble(),
                            ijb[constants::json::packer::MAX_WEIGHT].asDouble(),
                            gravityStrength,
                            masterItemRegister);

    // Create items and add them to the master register
    for (int x=iji.size(); x--;) {
        Item i(x,
                iji[x][constants::json::item::ID].asString(),
                iji[x][constants::json::item::WIDTH].asDouble(),
                iji[x][constants::json::item::DEPTH].asDouble(),
                iji[x][constants::json::item::HEIGHT].asDouble(),
                iji[x][constants::json::item::WEIGHT].asDouble(),
                iji[x][constants::json::item::ITEM_CONS_KEY].asString(),
                iji[x][constants::json::item::ALLOWED_ROTATIONS].asString());

        PackingProcessor.masterItemRegister_->fillItemRegisters(i);
    };

    // Split items by consolidation key and sort them on volume.
    PackingProcessor.CreateFinalSortedItemConsKeyVectors();
    for(auto& sortedItemConsKeyVector : PackingProcessor.GetItemConsKeyVectorsToBePacked()){
        PackingProcessor.startPacking(sortedItemConsKeyVector);
    };

    /* Just used when i want to time code execution. */
    // auto stop = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    // std::cout << duration.count() << std::endl;

    
    // Init the outgoing json builder
    jsonResponseBuilder outgoingJsonBuilder = jsonResponseBuilder(jsonPrecision, includeBins, includeItems, itemDimensionsAfter);
    outgoingJsonBuilder.generate(PackingProcessor);

    // std::ofstream myfile;
    // myfile.open ("ex1.txt");
    // std::string output = Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage());
    // myfile << output;
    // myfile.close();

    return strdup(Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage()).c_str());
    
    // return 0;

    };
};

extern "C" {
const void packToBinAlgorhitmFreeMemory(char *outputPtr) {
    free(outputPtr);
    };
};