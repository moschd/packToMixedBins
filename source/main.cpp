
/*
Get rid of ints. item positions work with integers, integer comparison is more reliable.
*/
#define MULTIPLIER 10000

/*
Set a base item key, used in the homogenous layer builder.
*/
#define BASE_ITEM_KEY 0

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
#define COMPILE_TO_SHARED_OBJECT_FILE true

#if !COMPILE_TO_SHARED_OBJECT_FILE
#define LOCAL_FOLDER "/home/dennis/packingOptimizerCompany/algorithms/packToBin"
#define LOCAL_INPUT_FILE LOCAL_FOLDER "/testfiles/test1.json"
#define LOCAL_OUTPUT_FILE LOCAL_FOLDER "/output.json"
#endif

/*
Include necessary files.
*/
#include <memory>
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
#include "itemPositionConstructor.h"
#include "packingCluster.h"
#include "packer.h"
#include "outgoingJsonBuilder.h"
#include "packingResultEvaluator.h"

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

        std::shared_ptr<ItemRegister> itemRegister = std::make_shared<ItemRegister>(incomingJsonBin[constants::json::inbound::bin::SORT_METHOD].asString(),
                                                                                    incomingJsonItems.size());
        std::shared_ptr<Gravity> masterGravity = std::make_shared<Gravity>(incomingJsonBin[constants::json::inbound::bin::GRAVITY_STRENGTH].asDouble(),
                                                                           itemRegister);
        std::shared_ptr<RequestedBin> requestedBin = std::make_shared<RequestedBin>(incomingJsonBin[constants::json::inbound::bin::TYPE].asString(),
                                                                                    incomingJsonBin[constants::json::inbound::bin::WIDTH].asDouble() * MULTIPLIER,
                                                                                    incomingJsonBin[constants::json::inbound::bin::DEPTH].asDouble() * MULTIPLIER,
                                                                                    incomingJsonBin[constants::json::inbound::bin::HEIGHT].asDouble() * MULTIPLIER,
                                                                                    incomingJsonBin[constants::json::inbound::bin::MAX_WEIGHT].asDouble(),
                                                                                    incomingJsonBin[constants::json::inbound::bin::PACKING_DIRECTION].asString());

        Packer packingProcessor(std::make_shared<PackingContext>(masterGravity, itemRegister, requestedBin));

        /* Initialize items and add them to the master register */
        for (int idx = incomingJsonItems.size(); idx--;)
        {
            packingProcessor.getContext()->getItemRegister()->addItem(
                std::make_shared<Item>(idx,
                                       incomingJsonItems[idx][constants::json::item::ID].asString(),
                                       incomingJsonItems[idx][constants::json::item::WIDTH].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::DEPTH].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::HEIGHT].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::WEIGHT].asDouble(),
                                       incomingJsonItems[idx][constants::json::item::ITEM_CONS_KEY].asString(),
                                       incomingJsonItems[idx][constants::json::item::ALLOWED_ROTATIONS].asString(),
                                       incomingJsonItems[idx][constants::json::item::GRAVITY_STRENGTH].asDouble()));
        };

        /* Split items by consolidation key and start packing. */
        for (const std::vector<int> sortedItemConsKeyVector : packingProcessor.getContext()->getItemRegister()->getSortedItemConsKeyVectors())
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
    std::cout << "----\n\n";
    std::unique_ptr<PackingResultEvaluator> evaluator = std::make_unique<PackingResultEvaluator>(packingProcessor);

    return 1;
};
#endif