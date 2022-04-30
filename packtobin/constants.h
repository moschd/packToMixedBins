#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>
#include <regex>

// namespace to hold constants
namespace constants
{

    constexpr int AxisWidth { 0 };
    constexpr int AxisDepth { 1 };
    constexpr int AxisHeight { 2 };

    constexpr std::array<int,3> AllAxis = {AxisWidth, AxisDepth, AxisHeight};
    constexpr std::array<double,3> START_POSITION = { 0, 0, 0 };

    constexpr int RT_WDH { 0 };
    constexpr int RT_DWH { 1 };
    constexpr int RT_HDW { 2 };
    constexpr int RT_DHW { 3 };
    constexpr int RT_WHD { 4 };
    constexpr int RT_HWD { 5 };

    constexpr std::array<int,6> AllRotationTypes = {RT_WDH, RT_DWH, RT_HDW, RT_DHW, RT_WHD, RT_HWD};
    // constexpr std::string validRotationType "^[0-5]$";

}
#endif