#define CYLINDER_SUPPORT false
/*
Always set to 0, cylinders are not supported (yet).
*/

#define COMPILE_TO_SO true
/*
Compile to a shared object file.
*/

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <cmath>
#include <regex>
#include <ext/pb_ds/assoc_container.hpp>
#include <jsoncpp/json/json.h>
#include "constants.h"
#include "geometry.h"
#include "kd-tree.h"
#include "geometricShape.h"
#include "item.h"
#include "itemSortMethods.h"
#include "itemregister.h"
#include "requestedBin.h"
#include "binCalculationCache.h"
#include "gravity.h"
#include "bin.h"
#include "packingCluster.h"
#include "packer.h"
#include "binSortMethods.h"
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

#if COMPILE_TO_SO
extern "C"
{
    char *packToBinAlgorithm(char *result,
                             const int bufferSize,
                             const char *incomingJson,
                             const bool includeBins = true,
                             const bool includeItems = true,
                             const bool itemDimensionsAfter = false,
                             const int responsePrecision = 7)
    {

#else
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    const bool includeBins = true;
    const bool includeItems = false;
    const bool itemDimensionsAfter = false;
    const int responsePrecision = 7;
    std::ifstream incomingJson("/home/dennis/po/algos/packToBin/testfiles/demo2.json");
#endif

        Json::Reader reader;
        Json::Value inboundRoot;
        reader.parse(incomingJson, inboundRoot);

        const Json::Value incomingJsonBin = inboundRoot[constants::json::inbound::bin::BIN];
        const Json::Value incomingJsonItems = inboundRoot[constants::json::inbound::item::ITEMS];

        ItemRegister masterItemRegister(incomingJsonBin[constants::json::inbound::bin::SORT_METHOD].asString());

        Gravity masterGravity(incomingJsonBin[constants::json::inbound::bin::GRAVITY_STRENGTH].asDouble());

        RequestedBin requestedBin(incomingJsonBin[constants::json::inbound::bin::TYPE].asString(),
                                  incomingJsonBin[constants::json::inbound::bin::WIDTH].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::DEPTH].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::HEIGHT].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::MAX_WEIGHT].asDouble());

        Packer packingProcessor(requestedBin,
                                masterGravity,
                                masterItemRegister,
                                incomingJsonBin[constants::json::inbound::bin::DISTRIBUTE_ITEMS].asBool());

        /* Initialize items and add them to the master register */
        for (int idx = incomingJsonItems.size(); idx--;)
        {
            Item i(idx,
                   incomingJsonItems[idx][constants::json::item::ID].asString(),
                   incomingJsonItems[idx][constants::json::item::WIDTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::DEPTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::HEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::DIAMETER].asDouble(),
                   incomingJsonItems[idx][constants::json::item::WEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::ITEM_CONS_KEY].asString(),
                   incomingJsonItems[idx][constants::json::item::ALLOWED_ROTATIONS].asString(),
                   incomingJsonItems[idx][constants::json::item::GRAVITY_STRENGTH].asDouble());

            packingProcessor.masterItemRegister_->addItem(i);
        };

        /* Split items by consolidation key and start packing. */
        for (auto &sortedItemConsKeyVector : packingProcessor.masterItemRegister_->getAllSortedItemConsKeyVectors())
        {
            packingProcessor.startPackingCluster(sortedItemConsKeyVector);
        };

        // /* Initialize outgoing json builder */
        ResponseBuilder outgoingJsonBuilder(responsePrecision, includeBins, includeItems, itemDimensionsAfter);
        outgoingJsonBuilder.generate(packingProcessor);

#if COMPILE_TO_SO
        std::string resultJson(Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage()));
        if (resultJson.size() < bufferSize)
        {
            resultJson.copy(result, bufferSize);
            return result;
        }
        else
        {
            return 0;
        }
    };
};
#else
    std::ofstream myfile;
    myfile.open("output.json");
    std::string output = Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage());
    myfile << output;
    myfile.close();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << std::endl;
    return 1;
};
#endif