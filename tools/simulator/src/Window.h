#pragma once

#include <cstdint>
#include <functional>
#include <string>

// A lamp drawn as LEDs on a dark panel, sized from the device's pixel pitch.
struct LampWindow
{
    std::string title;
    unsigned columns{0U};
    unsigned rows{0U};
    double widthMm{0.0};
    double heightMm{0.0};
    double scale{1.6}; // points per mm
    unsigned fps{30U};
    std::function<bool()> step;                     // advances the simulation one frame; false to quit
    std::function<uint8_t(unsigned, unsigned)> pixel; // brightness at column, row
};

#ifdef __APPLE__
int runWindow(const LampWindow &lamp);
bool savePng(const LampWindow &lamp, const char *path);
#endif
