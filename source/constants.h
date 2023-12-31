#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    constexpr std::array<int, 3> START_POSITION = {0, 0, 0};

    namespace shape
    {
        const std::string CUBOID = "CUBOID";
        const std::string CYLINDER = "CYLINDER";
    }

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
    }

    namespace distributor
    {
        namespace type
        {
            const std::string VOLUME = "VOLUME";
            const std::string WEIGHT = "WEIGHT";
        }
    }

    namespace axis
    {
        constexpr const unsigned int WIDTH{0};
        constexpr const unsigned int DEPTH{1};
        constexpr const unsigned int HEIGHT{2};
        constexpr const std::array<int, 3> ALL = {WIDTH, DEPTH, HEIGHT};

        namespace cylinder
        {
            constexpr const unsigned int BASE{10};
            constexpr const unsigned int X{11};
            constexpr const unsigned int Y{12};
            constexpr const unsigned int XY{13};
        }
    }

    namespace binComposer
    {
        namespace minimizationStrategy
        {
            constexpr const unsigned int VOLUME{10};
            constexpr const unsigned int BINS{20};
            constexpr const unsigned int STRICT_BINS{30};
        }
    }

    namespace item
    {
        namespace parameter
        {
            const std::string ALLOW_ALL = "ALLOWALL";
            const std::string NO_ITEMS_ON_TOP = "NOITEMSONTOP";
            const std::string BOTTOM_NO_ITEMS_ON_TOP = "BOTTOMNOITEMSONTOP";
        };
    }

    constexpr const unsigned int R{constants::axis::ALL.size()};

    namespace itemRegister
    {

        namespace itemConsolidationKey
        {
            const std::string PO_INTERNAL_ALL = "PO_INTERNAL_ALL";
        };

        namespace parameter
        {
            namespace sortMethod
            {
                const std::string VOLUME = "VOLUME";
                const std::string WEIGHT = "WEIGHT";
                const std::string OPTIMIZED = "OPTIMIZED";
            }
        }
    }

    namespace bin
    {
        namespace parameter
        {
            const std::string BOTTOM_UP_TEXT = "BOTTOMUP";
            const std::string BACK_TO_FRONT_TEXT = "BACKTOFRONT";
            const std::array<int, 3> BOTTOM_UP_ARRAY = {constants::axis::WIDTH, constants::axis::DEPTH, constants::axis::HEIGHT};
            const std::array<int, 3> BACK_TO_FRONT_ARRAY = {constants::axis::WIDTH, constants::axis::HEIGHT, constants::axis::DEPTH};

        }
    }

    namespace json
    {
        namespace inbound
        {
            namespace bins
            {
                const std::string BINS = "bins";
                const std::string TYPE = "type";
                const std::string WIDTH = "width";
                const std::string DEPTH = "depth";
                const std::string HEIGHT = "height";
                const std::string MAX_WEIGHT = "maxWeight";
                const std::string SORT_METHOD = "sortMethod";
                const std::string GRAVITY_STRENGTH = "gravityStrength";
                const std::string PACKING_DIRECTION = "packingDirection";
                const std::string NR_OF_AVAILABLE_BINS = "nrOfAvailableBins";
                const std::string ITEM_LIMIT = "itemLimit";
            };

            namespace item
            {
                const std::string ITEMS = "items";
            }
        };

        namespace outbound
        {
            const std::string PACKED_BINS = "packedBins";

            namespace header
            {
                const std::string REQUIRED_NR_OF_BINS = "requiredNrOfBins";
                const std::string TOTAL_VOLUME_UTIL = "totalVolumeUtil";
                const std::string TOTAL_WEIGHT_UTIL = "totalWeightUtil";
                const std::string UNFITTED_ITEMS = "unfittedItems";
            };
            namespace bin
            {
                const std::string ID = "id";
                const std::string TYPE = "type";
                const std::string NR_OF_ITEMS = "nrOfItems";
                const std::string WIDTH = "width";
                const std::string DEPTH = "depth";
                const std::string HEIGHT = "height";
                const std::string MAX_VOLUME = "maxVolume";
                const std::string ACTUAL_VOLUME = "actualVolume";
                const std::string ACTUAL_VOLUME_UTIL = "actualVolumeUtil";
                const std::string MAX_WEIGHT = "maxWeight";
                const std::string ACTUAL_WEIGHT = "actualWeight";
                const std::string ACTUAL_WEIGHT_UTIL = "actualWeightUtil";
                const std::string FITTED_ITEMS = "fittedItems";
                const std::string FURTHEST_POINT_WIDTH = "furthestPointWidth";
                const std::string FURTHEST_POINT_DEPTH = "furthestPointDepth";
                const std::string FURTHEST_POINT_HEIGHT = "furthestPointHeight";
            }
        };

        namespace item
        {
            const std::string ID = "id";
            const std::string SHAPE = "shape";
            const std::string WIDTH = "width";
            const std::string DEPTH = "depth";
            const std::string HEIGHT = "height";
            const std::string DIAMETER = "diameter";
            const std::string WEIGHT = "weight";
            const std::string VOLUME = "volume";
            const std::string ITEM_CONS_KEY = "itemConsKey";
            const std::string ALLOWED_ROTATIONS = "allowedRotations";
            const std::string X_COORDINATE = "xCoordinate";
            const std::string Y_COORDINATE = "yCoordinate";
            const std::string Z_COORDINATE = "zCoordinate";
            const std::string ROTATION_TYPE = "rotationType";
            const std::string ROTATION_TYPE_DESCRIPTION = "rotationTypeDescription";
            const std::string GRAVITY_STRENGTH = "gravityStrength";
            const std::string STACKING_STYLE = "stackingStyle";
            const std::string COMPATIBLE_BINS = "compatibleBins";

            namespace stackingStyle
            {
                const std::string ALLOW_ALL = "allowAll";
                const std::string NO_ITEMS_ON_TOP = "noItemsOnTop";
                const std::string BOTTOM_NO_ITEMS_ON_TOP = "bottomNoItemsOnTop";
            };

        };
    };
}
#endif