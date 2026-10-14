#if MODE_CHECKERBOARD

#include "modes/CheckerboardMode.h"

#include "services/DisplayService.h"

#include <algorithm>

static_assert(GRID_COLUMNS >= 4U, __STRING(MODE_CHECKERBOARD) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 4U, __STRING(MODE_CHECKERBOARD) " is not compatible with this device's display size.");

/**
 * @brief Resets the pattern state and clears the display frame.
 */
void CheckerboardMode::begin()
{
    inverted = false;
    offset = 0U;
    phase = 0U;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Cross-fades the lit and unlit squares of the checkerboard.
 *
 * After each full fade the pattern is inverted, and occasionally shifted by one pixel.
 */
void CheckerboardMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            const uint8_t squareX{static_cast<uint8_t>((x + offset) / squareSize)};
            const uint8_t squareY{static_cast<uint8_t>((y + offset) / squareSize)};
            const bool lit{(((squareX + squareY) % 2U) == 0U) != inverted};
            Display.setPixel(x, y, lit ? phase : static_cast<uint8_t>(UINT8_MAX - phase));
        }
    }
    if (phase < UINT8_MAX)
    {
        phase = static_cast<uint8_t>(std::min<int>(phase + step, UINT8_MAX));
    }
    else
    {
        phase = 0U;
        inverted = !inverted;
        if (random(100) > 70)
        {
            offset = static_cast<uint8_t>((offset + 1U) % (squareSize * 2U));
        }
    }
}

#endif // MODE_CHECKERBOARD
