#if MODE_WAVE

#include "modes/WaveMode.h"

#include "services/DisplayService.h"

#include <cmath>
#include <numbers>

static_assert(GRID_COLUMNS >= 1U, __STRING(MODE_WAVE) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 1U, __STRING(MODE_WAVE) " is not compatible with this device's display size.");

/**
 * @brief Resets the wave phase and clears the display frame.
 */
void WaveMode::begin()
{
    phase = 0.0F;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Renders the sum of three overlapping sine waves as per-pixel brightness, then advances the phase.
 */
void WaveMode::handle()
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
            const float fx{static_cast<float>(x)};
            const float fy{static_cast<float>(y)};
            const float wave1{std::sin((fx * 0.5F) + phase)};
            const float wave2{std::sin((fy * 0.4F) + (phase * 1.3F))};
            const float wave3{std::cos(((fx + fy) * 0.3F) - (phase * 0.8F))};
            const float combined{(wave1 + wave2 + wave3) / 3.0F};
            Display.setPixel(x, y, static_cast<uint8_t>(((combined + 1.0F) / 2.0F) * static_cast<float>(UINT8_MAX)));
        }
    }
    constexpr float wrap{2.0F * std::numbers::pi_v<float> * 100.0F};
    phase += speed;
    if (phase > wrap)
    {
        phase -= wrap;
    }
}

#endif // MODE_WAVE
