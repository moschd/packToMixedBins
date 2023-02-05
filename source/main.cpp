/*
Default parameters.
*/
#define DEFAULT_INCLUDE_BINS true
#define DEFAULT_INCLUDE_ITEMS true
#define DEFAULT_ITEM_DIMENSIONS_AFTER false
#define DEFAULT_RESPONSE_PRECISION 7

/*
Compile to a shared object file.
*/
#define COMPILE_TO_SHARED_OBJECT_FILE false

#if !COMPILE_TO_SHARED_OBJECT_FILE
#define LOCAL_FOLDER "/home/dennis/po/algos/packToBin"
#define LOCAL_INPUT_FILE LOCAL_FOLDER "/testfiles/demo.json"
#define LOCAL_OUTPUT_FILE LOCAL_FOLDER "/output.json"
#endif

/*
Include necessary files.
*/
#include <iostream>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <ext/pb_ds/assoc_container.hpp>
#include <jsoncpp/json/json.h>
#include "constants.h"
#include "geometry.h"
#include "kd-tree.h"
#include "geometricShape.h"
#include "item.h"
#include "itemSortMethods.h"
#include "packingContext.h"
#include "binCalculationCache.h"
#include "bin.h"
#include "packingCluster.h"
#include "packer.h"
#include "binSortMethods.h"
#include "outgoingJsonBuilder.h"

/*
Driver code.
*/
#if COMPILE_TO_SHARED_OBJECT_FILE
extern "C"
{
    char *packToBinAlgorithm(char *result,
                             const int bufferSize,
                             const char *incomingJson,
                             const bool includeBins = DEFAULT_INCLUDE_BINS,
                             const bool includeItems = DEFAULT_INCLUDE_ITEMS,
                             const bool itemDimensionsAfter = DEFAULT_ITEM_DIMENSIONS_AFTER,
                             const int responsePrecision = DEFAULT_RESPONSE_PRECISION)
    {

#else
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    const bool includeBins = DEFAULT_INCLUDE_BINS;
    const bool includeItems = DEFAULT_INCLUDE_ITEMS;
    const bool itemDimensionsAfter = DEFAULT_ITEM_DIMENSIONS_AFTER;
    const int responsePrecision = DEFAULT_RESPONSE_PRECISION;
    std::ifstream incomingJson(LOCAL_INPUT_FILE);
#endif

        Json::Reader reader;
        Json::Value inboundRoot;
        reader.parse(incomingJson, inboundRoot);

        const Json::Value incomingJsonBin = inboundRoot[constants::json::inbound::bin::BIN];
        const Json::Value incomingJsonItems = inboundRoot[constants::json::inbound::item::ITEMS];

        ItemRegister itemRegister(incomingJsonBin[constants::json::inbound::bin::SORT_METHOD].asString(),
                                  incomingJsonItems.size());

        Gravity masterGravity(incomingJsonBin[constants::json::inbound::bin::GRAVITY_STRENGTH].asDouble());

        RequestedBin requestedBin(incomingJsonBin[constants::json::inbound::bin::TYPE].asString(),
                                  incomingJsonBin[constants::json::inbound::bin::WIDTH].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::DEPTH].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::HEIGHT].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::MAX_WEIGHT].asDouble(),
                                  incomingJsonBin[constants::json::inbound::bin::PACKING_DIRECTION].asString());

        PackingContext context(masterGravity, itemRegister, requestedBin);

        Packer packingProcessor(context);

        /* Initialize items and add them to the master register */
        for (int idx = incomingJsonItems.size(); idx--;)
        {
            Item i(idx,
                   incomingJsonItems[idx][constants::json::item::ID].asString(),
                   incomingJsonItems[idx][constants::json::item::WIDTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::DEPTH].asDouble(),
                   incomingJsonItems[idx][constants::json::item::HEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::WEIGHT].asDouble(),
                   incomingJsonItems[idx][constants::json::item::ITEM_CONS_KEY].asString(),
                   incomingJsonItems[idx][constants::json::item::ALLOWED_ROTATIONS].asString(),
                   incomingJsonItems[idx][constants::json::item::GRAVITY_STRENGTH].asDouble());

            packingProcessor.getModifiableContext()->addItemToRegister(i);
        };

        /* Split items by consolidation key and start packing. */
        for (auto &sortedItemConsKeyVector : packingProcessor.getContext()->getSortedItemConsKeyVectors())
        {
            packingProcessor.startPackingCluster(sortedItemConsKeyVector);
        };

        // /* Initialize outgoing json builder */
        ResponseBuilder outgoingJsonBuilder(responsePrecision, includeBins, includeItems, itemDimensionsAfter);
        outgoingJsonBuilder.generate(packingProcessor);

        std::string resultJson(Json::writeString(outgoingJsonBuilder.getBuilder(), outgoingJsonBuilder.getMessage()));

#if COMPILE_TO_SHARED_OBJECT_FILE
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
    myfile.open(LOCAL_OUTPUT_FILE);
    myfile << resultJson;
    myfile.close();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << std::endl;
    return 1;
};
#endif