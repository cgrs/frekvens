#pragma once

#include "SimDevice.h"
#include "config/constants.h"
#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <span>

// Frame buffer and drawing helpers of the real DisplayService; the drawing code is extracted from
// firmware/src/services/DisplayService.cpp at build time, so only the hardware side is replaced.
class DisplayService
{
public:
    enum class Orientation : uint8_t // NOLINT(performance-enum-size)
    {
        deg0,
        deg90,
        deg180,
        deg270,
    };

private:
    Orientation orientation{Orientation::deg0};

public:
    static inline std::array<size_t, GRID_COLUMNS * GRID_ROWS> pixels{simDevice->ledMap};

    bool render{false};
    std::array<uint8_t, GRID_COLUMNS * GRID_ROWS> frame{};
    unsigned long flushes{0UL};

    [[nodiscard]] Orientation getOrientation() const { return orientation; }
    [[nodiscard]] float getRatio() const { return static_cast<float>(simDevice->pitchHorizontal) / static_cast<float>(simDevice->pitchVertical); }

    void getFrame(std::span<uint8_t, GRID_COLUMNS * GRID_ROWS> _frame) const;
    void setFrame(std::span<const uint8_t, GRID_COLUMNS * GRID_ROWS> _frame);

    [[nodiscard]] uint8_t getPixel(size_t idx) const;
    [[nodiscard]] uint8_t getPixel(uint8_t x, uint8_t y) const;

    void setPixel(size_t idx, uint8_t _brightness);
    void setPixel(uint8_t x, uint8_t y, uint8_t _brightness);

    void drawEllipseOutline(float x, float y, float radius, uint8_t _brightness);
    void drawEllipseSolid(float x, float y, float radius, uint8_t _brightness);

    void drawRectangleOutline(size_t minX, size_t columns, size_t minY, size_t maxY, uint8_t _brightness);
    void drawRectangleSolid(size_t minX, size_t columns, size_t minY, size_t maxY, uint8_t _brightness);

    void drawLineHorizontal(size_t xMin, size_t columns, size_t y, uint8_t _brightness);
    void drawLineVertical(uint8_t x, uint8_t yMin, uint8_t yMax, uint8_t _brightness);

    void fillColumn(uint8_t x, uint8_t _brightness);
    void fillFrame(uint8_t _brightness);
    void fillRow(size_t y, uint8_t _brightness);
    void fillRows(size_t minY, size_t rows, uint8_t _brightness);

    void flush() { ++flushes; }

    void invertFrame();
};

extern DisplayService &Display;
