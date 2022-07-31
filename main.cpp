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

extern "C"
{
    const char *packToBinAlgorithm(char *incomingJson,
                                   bool includeBins = 1,
                                   bool includeItems = 1,
                                   bool itemDimensionsAfter = 0,
                                   int responsePrecision = 7,
                                   double gravityStrength = 0.0,
                                   int mainSortMethod = 1)
    {

        // int main()
        // {
        //     auto start = std::chrono::high_resolution_clock::now();
        //     double gravityStrength = 0.0;
        //     bool itemDimensionsAfter = 0;
        //     bool includeItems = 1;
        //     bool includeBins = 1;
        //     int mainSortMethod = 1;
        //     int responsePrecision = 7;
        // std::ifstream incomingJson("/home/moschd/pomaster/packingOptimizerAlgos/packToBin/testfiles/nullutil.json");

        // std::ifstream incomingJson("/home/moschd/pomaster/packingOptimizerAlgos/packToBin/testfiles/testjson.json");
        // std::ifstream incomingJson ("/home/moschd/packingOptimizerAlgos/packToBin/testfiles/testjson copy 2.json");
        // std::ifstream incomingJson ("/home/moschd/packingOptimizerAlgos/packToBin/testfiles/cpp_vs_python_1000_items.json");
        // std::ifstream incomingJson ("/home/moschd/packingOptimizerAlgos/packToBin/testfiles/cpp_vs_python_2500_items.json");
        // std::ifstream incomingJson ("/home/moschd/pomaster/packingOptimizerAlgos/packToBin/testfiles/cpp_vs_python_5000_items.json");
        // std::ifstream incomingJson ("/home/moschd/packingOptimizerAlgos/packToBin/testfiles/cpp_vs_python_10000_items.json");

        Json::Reader reader;
        Json::Value inboundRoot;
        reader.parse(incomingJson, inboundRoot);
        Json::Value incomingJsonBin = inboundRoot[constants::json::inbound::BIN];
        Json::Value incomingJsonItems = inboundRoot[constants::json::inbound::ITEMS];

        // Init item register
        ItemRegister masterItemRegister(mainSortMethod);

        // Init gravity
        Gravity masterGravity(gravityStrength);

        // Init Packer
        Packer PackingProcessor(incomingJsonBin[constants::json::packer::TYPE].asString(),
                                incomingJsonBin[constants::json::packer::WIDTH].asDouble(),
                                incomingJsonBin[constants::json::packer::DEPTH].asDouble(),
                                incomingJsonBin[constants::json::packer::HEIGHT].asDouble(),
                                incomingJsonBin[constants::json::packer::MAX_WEIGHT].asDouble(),
                                masterGravity,
                                masterItemRegister);

        /* Init items and add them to the master register */
        for (int idx = incomingJsonItems.size(); idx--;)
        {
            Item i(idx,
                   incomingJsonItems[idx][constants::json::item::ID].asString(),
                   incomingJsonItems[idx][constants::json::item::WIDTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::DEPTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::HEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::WEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::ITEM_CONS_KEY].asString(),
                   incomingJsonItems[idx][constants::json::item::ALLOWED_ROTATIONS].asString());

            PackingProcessor.masterItemRegister_->addItem(i);
        };

        /* Split items by consolidation key, start packing for each of them. */
        for (auto &sortedItemConsKeyVector : PackingProcessor.masterItemRegister_->GetSortedItemConsKeyVectors())
        {
            PackingProcessor.startPacking(sortedItemConsKeyVector);
        };

        /* Init outgoing json builder */
        jsonResponseBuilder outgoingJsonBuilder(responsePrecision, includeBins, includeItems, itemDimensionsAfter);
        outgoingJsonBuilder.generate(PackingProcessor);

        // std::ofstream myfile;
        // myfile.open("ex1.txt");
        // std::string output = Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage());
        // myfile << output;
        // myfile.close();

        /* Just used when i want to time code execution. */
        // auto stop = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        // std::cout << duration.count() << std::endl;

        return strdup(Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage()).c_str());

        // return 0;
    };
};

extern "C"
{
    const void packToBinAlgorithmFreeMemory(char *outputPtr)
    {
        free(outputPtr);
    };
};