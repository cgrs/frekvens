#if MODE_SPIRAL

#include "modes/SpiralMode.h"

#include "services/DisplayService.h"

#include <cmath>
#include <numbers>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_SPIRAL) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_SPIRAL) " is not compatible with this device's display size.");

/**
 * @brief Resets the spiral to its center and clears the display frame.
 */
void SpiralMode::begin()
{
    angle = 0.0F;
    radius = 0.0F;
    expanding = true;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Fades the previous frame and draws a short rotating spiral arm that breathes in and out.
 */
void SpiralMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    for (size_t idx{0U}; idx < GRID_COLUMNS * GRID_ROWS; ++idx)
    {
        const uint8_t brightness{Display.getPixel(idx)};
        Display.setPixel(idx, brightness > 20U ? static_cast<uint8_t>(brightness - 20U) : 0U);
    }
    for (uint8_t i{0U}; i < points; ++i)
    {
        const float currentAngle{angle + (static_cast<float>(i) * 0.5F)};
        const float currentRadius{radius - (static_cast<float>(i) * 0.3F)};
        if (currentRadius >= 0.0F)
        {
            const int x{centerX + static_cast<int>(std::cos(currentAngle) * currentRadius)};
            const int y{centerY + static_cast<int>(std::sin(currentAngle) * currentRadius)};
            if (x >= 0 && x < static_cast<int>(GRID_COLUMNS) && y >= 0 && y < static_cast<int>(GRID_ROWS))
            {
                Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y),
                                 static_cast<uint8_t>(UINT8_MAX - (i * 40U)));
            }
        }
    }
    constexpr float turns{2.0F * std::numbers::pi_v<float> * 10.0F};
    angle += 0.15F;
    if (angle > turns)
    {
        angle -= turns;
    }
    if (expanding)
    {
        radius += 0.1F;
        if (radius >= maxRadius)
        {
            expanding = false;
        }
    }
    else
    {
        radius -= 0.1F;
        if (radius <= minRadius)
        {
            expanding = true;
        }
    }
}

#endif // MODE_SPIRAL
