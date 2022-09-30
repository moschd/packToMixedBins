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
#include "calculationCache.h"
#include "gravity.h"
#include "bin.h"
#include "packingCluster.h"
#include "packer.h"
#include "binSortMethods.h"
#include "distributor.h"
#include "outgoingJsonBuilder.h"

#if __cplusplus < 201703
#define CYLINDER_SUPPORT 0
#else
#define CYLINDER_SUPPORT 1
#endif

/*
    ----------------------------------------------------
                        PACK TO BIN
    ----------------------------------------------------
    Input:
        JSON
    Output:
        JSON
*/

#define COMPILE_TO_SO 1

#if COMPILE_TO_SO
extern "C"
{
    char *packToBinAlgorithm(char *result,
                             const int bufferSize,
                             const char *incomingJson,
                             const bool includeBins = 1,
                             const bool includeItems = 1,
                             const bool itemDimensionsAfter = 0,
                             const int responsePrecision = 7)
    {

#else
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    const bool itemDimensionsAfter = 0;
    const bool includeItems = 1;
    const bool includeBins = 1;
    const int responsePrecision = 7;
    std::ifstream incomingJson("/home/dennis/po/algos/packToBin/testfiles/5000_items.json");
#endif

        Json::Reader reader;
        Json::Value inboundRoot;
        reader.parse(incomingJson, inboundRoot);

        const Json::Value incomingJsonBin = inboundRoot[constants::json::inbound::bin::BIN];
        const Json::Value incomingJsonItems = inboundRoot[constants::json::inbound::item::ITEMS];

        ItemRegister masterItemRegister(incomingJsonBin[constants::json::inbound::bin::SORT_METHOD].asString());
        Gravity masterGravity(incomingJsonBin[constants::json::inbound::bin::GRAVITY_STRENGTH].asDouble());
        Packer packingProcessor(incomingJsonBin[constants::json::inbound::bin::TYPE].asString(),
                                incomingJsonBin[constants::json::inbound::bin::WIDTH].asDouble(),
                                incomingJsonBin[constants::json::inbound::bin::DEPTH].asDouble(),
                                incomingJsonBin[constants::json::inbound::bin::HEIGHT].asDouble(),
                                incomingJsonBin[constants::json::inbound::bin::MAX_WEIGHT].asDouble(),
                                masterGravity,
                                masterItemRegister,
                                incomingJsonBin[constants::json::inbound::bin::DISTRIBUTE].asString());

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
            packingProcessor.startPacking(sortedItemConsKeyVector);
        };

        /* Check if item ditribution is requested */
        if (packingProcessor.requestsDistribution())
        {
            for (auto cluster : packingProcessor.getClusters())
            {
                if (cluster.getPackedBins().size() > 1)
                {
                    Distributor distributor(&cluster, packingProcessor.distribute_);
                    packingProcessor.setCluster(cluster.id_, distributor.packedCluster_);
                };
            };
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