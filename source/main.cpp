
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
#define DEFAULT_MINIMIZATION_STRATEGY 10

/*
Compile to a shared object file.
*/
#define COMPILE_TO_SHARED_OBJECT_FILE true
#define DEBUG !COMPILE_TO_SHARED_OBJECT_FILE

#if !COMPILE_TO_SHARED_OBJECT_FILE
#define LOCAL_FOLDER "/home/dennismosch/packingOptimizerCompany/algorithms/packToMixedBins"
#define LOCAL_INPUT_FILE LOCAL_FOLDER "/testfiles/demo.json"
#define LOCAL_OUTPUT_FILE LOCAL_FOLDER "/output.json"
#endif

/*
Include necessary files.
*/
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <ext/pb_ds/assoc_container.hpp>
#include "myjsoncpp/json/json.h"
#include "constants.h"
#include "geometry.h"
#include "kd-tree.h"
#include "geometricShape.h"
#include "item.h"
#include "packingContext.h"
#include "binCalculationCache.h"
#include "bin.h"
#include "itemPositionConstructor.h"
#include "packingCluster.h"
#include "packer.h"
#include "mixedBinPackerHandler.h"
#include "binComposer.h"
#include "outgoingJsonBuilder.h"
#include "packingResultEvaluator.h"

/*
Driver code.
*/
#if COMPILE_TO_SHARED_OBJECT_FILE
extern "C"
{
    char *packToMixedBinsAlgorithm(char *result,
                                   const int bufferSize,
                                   const char *incomingJson,
                                   const bool includeBins = DEFAULT_INCLUDE_BINS,
                                   const bool includeItems = DEFAULT_INCLUDE_ITEMS,
                                   const bool itemDimensionsAfter = DEFAULT_ITEM_DIMENSIONS_AFTER,
                                   const int responsePrecision = DEFAULT_RESPONSE_PRECISION,
                                   const int minimizationStrategy = DEFAULT_MINIMIZATION_STRATEGY)
    {

#else

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    const bool includeBins = DEFAULT_INCLUDE_BINS;
    const bool includeItems = DEFAULT_INCLUDE_ITEMS;
    const bool itemDimensionsAfter = DEFAULT_ITEM_DIMENSIONS_AFTER;
    const int responsePrecision = DEFAULT_RESPONSE_PRECISION;
    const int minimizationStrategy = DEFAULT_MINIMIZATION_STRATEGY;

    std::ifstream incomingJsonFile(LOCAL_INPUT_FILE);

    std::string line, text;
    while (std::getline(incomingJsonFile, line))
    {
        text += line + "\n";
    }
    const char *incomingJson = text.c_str();
#endif

        Json::Value inboundRoot;
        std::stringstream(incomingJson) >> inboundRoot;

        const Json::Value incomingJsonBins = inboundRoot[constants::json::inbound::bins::BINS];
        const Json::Value incomingJsonItems = inboundRoot[constants::json::inbound::item::ITEMS];

        std::shared_ptr<ItemRegister> itemRegister =
            std::make_shared<ItemRegister>(constants::itemRegister::parameter::sortMethod::OPTIMIZED,
                                           incomingJsonItems.size());

        std::shared_ptr<BinComposer> binComposer = std::make_shared<BinComposer>(itemRegister, minimizationStrategy);

        for (int idx = incomingJsonBins.size(); idx--;)
        {
            std::shared_ptr<RequestedBin> requestedBin =
                std::make_shared<RequestedBin>(incomingJsonBins[idx][constants::json::inbound::bins::TYPE].asString(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::WIDTH].asDouble() * MULTIPLIER,
                                               incomingJsonBins[idx][constants::json::inbound::bins::DEPTH].asDouble() * MULTIPLIER,
                                               incomingJsonBins[idx][constants::json::inbound::bins::HEIGHT].asDouble() * MULTIPLIER,
                                               incomingJsonBins[idx][constants::json::inbound::bins::MAX_WEIGHT].asDouble(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::NR_OF_AVAILABLE_BINS].asInt(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::ITEM_LIMIT].asInt(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::PACKING_DIRECTION].asString(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::SORT_METHOD].asString(),
                                               incomingJsonBins[idx][constants::json::inbound::bins::GRAVITY_STRENGTH].asDouble());

            binComposer->addRequestedBin(requestedBin);
        };

        /* Initialize items and add them to the master register */
        for (int idx = incomingJsonItems.size(); idx--;)
        {

            std::vector<std::string> compatibleBins = {};
            for (int compatibleBinIdx = incomingJsonItems[idx][constants::json::item::COMPATIBLE_BINS].size(); compatibleBinIdx--;)
            {
                compatibleBins.push_back(incomingJsonItems[idx][constants::json::item::COMPATIBLE_BINS][compatibleBinIdx].asString());
            };

            binComposer->getMasterItemRegister()->addItem(
                std::make_shared<Item>(idx + 1,
                                       incomingJsonItems[idx][constants::json::item::ID].asString(),
                                       incomingJsonItems[idx][constants::json::item::WIDTH].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::DEPTH].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::HEIGHT].asDouble() * MULTIPLIER,
                                       incomingJsonItems[idx][constants::json::item::WEIGHT].asDouble(),
                                       incomingJsonItems[idx][constants::json::item::ITEM_CONS_KEY].asString(),
                                       incomingJsonItems[idx][constants::json::item::ALLOWED_ROTATIONS].asString(),
                                       incomingJsonItems[idx][constants::json::item::GRAVITY_STRENGTH].asDouble(),
                                       compatibleBins,
                                       incomingJsonItems[idx][constants::json::item::STACKING_STYLE].asString()));

            binComposer->addItem(idx + 1);
        };

        binComposer->startPacking();

        /* Initialize outgoing json builder */
        ResponseBuilder outgoingJsonBuilder(responsePrecision, includeBins, includeItems, itemDimensionsAfter);
        outgoingJsonBuilder.generate(binComposer);
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
    // std::unique_ptr<PackingResultEvaluator> evaluator = std::make_unique<PackingResultEvaluator>(binComposer->getPacker());

    return 1;
};
#endif