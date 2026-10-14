#if MODE_RADAR

#include "modes/RadarMode.h"

#include "services/DisplayService.h"

#include <cmath>
#include <numbers>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_RADAR) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_RADAR) " is not compatible with this device's display size.");

/**
 * @brief Resets the sweep and blips, then clears the display frame.
 */
void RadarMode::begin()
{
    sweepAngle = 0.0F;
    for (Blip &blip : blips)
    {
        blip.x = static_cast<uint8_t>(random(GRID_COLUMNS));
        blip.y = static_cast<uint8_t>(random(GRID_ROWS));
        blip.age = 0U;
        blip.active = false;
    }
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Renders the rotating sweep, the range rings and the fading blips.
 *
 * Blips are lit at full brightness when the sweep passes them, then fade out and expire.
 */
void RadarMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    constexpr float twoPi{2.0F * std::numbers::pi_v<float>};
    for (size_t idx{0U}; idx < GRID_COLUMNS * GRID_ROWS; ++idx)
    {
        const uint8_t brightness{Display.getPixel(idx)};
        Display.setPixel(idx, brightness > 10U ? static_cast<uint8_t>(brightness - 10U) : 0U);
    }
    // Range rings
    for (uint8_t r{2U}; r < sweepRadius; r += 3U)
    {
        for (float a{0.0F}; a < twoPi; a += 0.3F)
        {
            const int x{centerX + static_cast<int>(std::cos(a) * static_cast<float>(r))};
            const int y{centerY + static_cast<int>(std::sin(a) * static_cast<float>(r))};
            if (x >= 0 && x < static_cast<int>(GRID_COLUMNS) && y >= 0 && y < static_cast<int>(GRID_ROWS))
            {
                Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), 30U);
            }
        }
    }
    // Sweep line
    for (uint8_t r{0U}; r < sweepRadius; ++r)
    {
        const int x{centerX + static_cast<int>(std::cos(sweepAngle) * static_cast<float>(r))};
        const int y{centerY + static_cast<int>(std::sin(sweepAngle) * static_cast<float>(r))};
        if (x >= 0 && x < static_cast<int>(GRID_COLUMNS) && y >= 0 && y < static_cast<int>(GRID_ROWS))
        {
            Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y),
                             static_cast<uint8_t>(UINT8_MAX - (r * 20U)));
        }
    }
    // Blips
    for (Blip &blip : blips)
    {
        if (!blip.active)
        {
            continue;
        }
        const float blipAngle{std::atan2(static_cast<float>(blip.y) - static_cast<float>(centerY),
                                         static_cast<float>(blip.x) - static_cast<float>(centerX))};
        float angleDiff{std::fabs(sweepAngle - blipAngle)};
        if (angleDiff > std::numbers::pi_v<float>)
        {
            angleDiff = twoPi - angleDiff;
        }
        if (angleDiff < 0.3F)
        {
            Display.setPixel(blip.x, blip.y, UINT8_MAX);
            blip.age = 0U;
        }
        else if (blip.age < 100U)
        {
            Display.setPixel(blip.x, blip.y, static_cast<uint8_t>(200U - (blip.age * 2U)));
            ++blip.age;
        }
        else
        {
            blip.active = false;
        }
    }
    // Spawn
    if (random(100) > 90)
    {
        for (Blip &blip : blips)
        {
            if (!blip.active)
            {
                blip.x = static_cast<uint8_t>(random(GRID_COLUMNS));
                blip.y = static_cast<uint8_t>(random(GRID_ROWS));
                blip.age = 0U;
                blip.active = true;
                break;
            }
        }
    }
    sweepAngle += sweepSpeed;
    if (sweepAngle > twoPi)
    {
        sweepAngle -= twoPi;
    }
}

#endif // MODE_RADAR
