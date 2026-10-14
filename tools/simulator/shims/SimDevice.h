#pragma once

#include "config/constants.h"

#include <array>
#include <cstddef>
#include <string_view>

/**
 * A lamp the simulator can show. The table is generated from firmware/include/config/Ikea*.h at build time.
 * The grid size stays a compile-time constant of the modes, so every profile must match the build's grid.
 */
struct SimDevice
{
    std::string_view id;
    std::string_view title;
    unsigned columns;
    unsigned rows;
    unsigned pitchHorizontal; // mm
    unsigned pitchVertical;   // mm
    std::array<size_t, GRID_COLUMNS * GRID_ROWS> ledMap; // wiring order of the LEDs, for PixelSequence
};

inline constexpr SimDevice simDevices[]{
#include "devices.inc"
};

inline const SimDevice *simDevice{[]
{
    for (const SimDevice &device : simDevices)
    {
        if (device.id == "obegransad")
        {
            return &device;
        }
    }
    return &simDevices[0U];
}()};
