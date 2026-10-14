#if MODE_SPOTLIGHT

#include "modes/SpotlightMode.h"

#include "services/DisplayService.h"

#include <cmath>

static_assert(GRID_COLUMNS >= 9U, __STRING(MODE_SPOTLIGHT) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 9U, __STRING(MODE_SPOTLIGHT) " is not compatible with this device's display size.");

/**
 * @brief Places the spotlight at a random position away from the edges, with a random velocity.
 */
void SpotlightMode::begin()
{
    x = static_cast<float>(random(margin, GRID_COLUMNS - margin));
    y = static_cast<float>(random(margin, GRID_ROWS - margin));
    const float magX{static_cast<float>(random(28, 37)) / 100.0F};
    const float magY{static_cast<float>(random(17, 26)) / 100.0F};
    vx = random(2) == 0 ? magX : -magX;
    vy = random(2) == 0 ? magY : -magY;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Moves the spotlight, bouncing off the display edges, and redraws its quadratic falloff.
 */
void SpotlightMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    constexpr float maxX{static_cast<float>(GRID_COLUMNS - 1U)};
    constexpr float maxY{static_cast<float>(GRID_ROWS - 1U)};
    x += vx;
    y += vy;
    if (x < 0.0F || x > maxX)
    {
        x = x < 0.0F ? 0.0F : maxX;
        vx = -vx;
    }
    if (y < 0.0F || y > maxY)
    {
        y = y < 0.0F ? 0.0F : maxY;
        vy = -vy;
    }
    Display.fillFrame(0U);
    for (uint8_t py{0U}; py < GRID_ROWS; ++py)
    {
        for (uint8_t px{0U}; px < GRID_COLUMNS; ++px)
        {
            const float dx{static_cast<float>(px) - x};
            const float dy{static_cast<float>(py) - y};
            const float distance{std::sqrt((dx * dx) + (dy * dy))};
            if (distance >= radius)
            {
                continue;
            }
            const float falloff{1.0F - (distance / radius)};
            const auto brightness{static_cast<int>((static_cast<float>(UINT8_MAX) * falloff * falloff) + 0.5F)};
            if (brightness >= 16)
            {
                Display.setPixel(px, py, static_cast<uint8_t>(brightness));
            }
        }
    }
    Display.setPixel(static_cast<uint8_t>(x + 0.5F), static_cast<uint8_t>(y + 0.5F), UINT8_MAX);
}

#endif // MODE_SPOTLIGHT
