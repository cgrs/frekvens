#if MODE_WAVEBARS

#include "modes/WaveBarsMode.h"

#include "services/DisplayService.h"

#include <cmath>

static_assert(GRID_COLUMNS >= 1U, __STRING(MODE_WAVEBARS) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 2U, __STRING(MODE_WAVEBARS) " is not compatible with this device's display size.");

/**
 * @brief Resets the wave phase and clears the display frame.
 */
void WaveBarsMode::begin()
{
    phase = 0.0F;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Draws one bottom-aligned bar per column, with heights following a moving sine wave.
 */
void WaveBarsMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    Display.fillFrame(0U);
    for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
    {
        const float wave{(std::sin(phase + (static_cast<float>(x) * 0.5F)) + 1.0F) * 0.5F};
        const auto height{static_cast<uint8_t>(wave * static_cast<float>(GRID_ROWS - 1U))};
        const auto brightness{static_cast<uint8_t>(120.0F + (wave * 120.0F))};
        for (uint8_t n{0U}; n <= height; ++n)
        {
            Display.setPixel(x, static_cast<uint8_t>(GRID_ROWS - 1U - n), brightness);
        }
    }
    phase += 0.2F;
}

#endif // MODE_WAVEBARS
