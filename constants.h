#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>
#include <regex>

// namespace to hold constants
namespace constants
{

    constexpr int AxisWidth{0};
    constexpr int AxisDepth{1};
    constexpr int AxisHeight{2};

    constexpr std::array<int, 3> AllAxis = {AxisWidth, AxisDepth, AxisHeight};
    constexpr std::array<double, 3> START_POSITION = {0, 0, 0};

    constexpr int R{constants::AllAxis.size()};

    constexpr int RT_WDH{0};
    constexpr int RT_DWH{1};
    constexpr int RT_HDW{2};
    constexpr int RT_DHW{3};
    constexpr int RT_WHD{4};
    constexpr int RT_HWD{5};

    namespace parameter
    {
        const int VOLUME = 1;
        const int WEIGHT = 2;
    }

    namespace json
    {
        namespace inbound
        {
            const std::string BIN = "bin";
            const std::string ITEMS = "items";
        };

        namespace outbound
        {
            const std::string BIN_DETAILS = "binDetails";
            const std::string PACKED_BINS = "packedBins";
            const std::string EXCEPTION = "exception";

            namespace header
            {
                const std::string REQUIRED_NR_OF_BINS = "requiredNrOfBins";
                const std::string TOTAL_VOLUME_UTIL = "totalVolumeUtil";
                const std::string TOTAL_WEIGHT_UTIL = "totalWeightUtil";
                const std::string UNFITTED_ITEMS = "unfittedItems";
            };
        };

        namespace packer
        {
            const std::string TYPE = "type";
            const std::string WIDTH = "width";
            const std::string DEPTH = "depth";
            const std::string HEIGHT = "height";
            const std::string MAX_WEIGHT = "maxWeight";
        };

        namespace item
        {
            const std::string ID = "id";
            const std::string WIDTH = "width";
            const std::string DEPTH = "depth";
            const std::string HEIGHT = "height";
            const std::string WEIGHT = "weight";
            const std::string VOLUME = "volume";
            const std::string ITEM_CONS_KEY = "itemConsKey";
            const std::string ALLOWED_ROTATIONS = "allowedRotations";
            const std::string X_COORDINATE = "xCoordinate";
            const std::string Y_COORDINATE = "yCoordinate";
            const std::string Z_COORDINATE = "zCoordinate";
            const std::string ROTATION_TYPE = "rotationType";
            const std::string ROTATION_TYPE_DESCRIPTION = "rotationTypeDescription";
        };

        namespace bin
        {
            const std::string ID = "id";
            const std::string TYPE = "type";
            const std::string NR_OF_ITEMS = "nrOfItems";

            const std::string MAX_WIDTH = "maxWidth";
            const std::string MAX_DEPTH = "maxDepth";
            const std::string MAX_HEIGHT = "maxHeight";

            const std::string MAX_VOLUME = "maxVolume";
            const std::string ACTUAL_VOLUME = "actualVolume";
            const std::string ACTUAL_VOLUME_UTIL = "actualVolumeUtil";

            const std::string MAX_WEIGHT = "maxWeight";
            const std::string ACTUAL_WEIGHT = "actualWeight";
            const std::string ACTUAL_WEIGHT_UTIL = "actualWeightUtil";

            const std::string FITTED_ITEMS = "fittedItems";

        }
    };

}
#endif