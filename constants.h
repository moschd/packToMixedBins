#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    constexpr std::array<double, 3> START_POSITION = {0, 0, 0};

    namespace rotation
    {
        namespace type
        {
            constexpr const unsigned int WDH{0}; // {width_, depth_, height_};
            constexpr const unsigned int DWH{1}; // {depth_, width_, height_};
            constexpr const unsigned int HDW{2}; // {height_, depth_, width_};
            constexpr const unsigned int DHW{3}; // {depth_, height_, width_};
            constexpr const unsigned int WHD{4}; // {width_, height_, depth_};
            constexpr const unsigned int HWD{5}; // {height_, width_, depth_};
        }
        namespace description
        {
            constexpr const std::array<char[76], 6> text = {"No box rotation",
                                                            "Rotate the box around the z-axis by 90°",
                                                            "Rotate the box around the x-axis by 90°",
                                                            "Rotate the box around the x-axis by 90° and then around the z-axis by 90°",
                                                            "Rotate the box around the y-axis by 90°",
                                                            "Rotate the box around the z-axis by 90° and then around the x-axis by 90°"};
        }
    }

    namespace axis
    {
        constexpr const unsigned int WIDTH{0};
        constexpr const unsigned int DEPTH{1};
        constexpr const unsigned int HEIGHT{2};
        constexpr const std::array<int, 3> ALL = {WIDTH, DEPTH, HEIGHT};
    }

    constexpr const unsigned int R{constants::axis::ALL.size()};

    namespace parameter
    {
        constexpr const unsigned int VOLUME = 1;
        constexpr const unsigned int WEIGHT = 2;
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