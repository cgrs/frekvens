#if MODE_BUBBLES

#include "modes/BubblesMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <cmath>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_BUBBLES) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_BUBBLES) " is not compatible with this device's display size.");

/**
 * @brief Gives a bubble a random position, starting radius, growth speed and brightness.
 *
 * @param bubble Bubble to reset.
 */
void BubblesMode::reset(Bubble &bubble)
{
    bubble.x = static_cast<float>(random(GRID_COLUMNS));
    bubble.y = static_cast<float>(random(GRID_ROWS));
    bubble.radius = static_cast<float>(random(1, 3));
    bubble.speed = 0.2F + (static_cast<float>(random(5)) / 10.0F);
    bubble.brightness = static_cast<uint8_t>(random(120, UINT8_MAX));
}

/**
 * @brief Randomizes all bubbles and clears the display frame.
 */
void BubblesMode::begin()
{
    for (Bubble &bubble : bubbles)
    {
        reset(bubble);
    }
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Grows each bubble ring and redraws the frame, resetting bubbles that grew too large.
 *
 * Where rings cross, the brighter one is kept.
 */
void BubblesMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    Display.fillFrame(0U);
    for (Bubble &bubble : bubbles)
    {
        bubble.radius += bubble.speed;
        if (bubble.radius > maxRadius)
        {
            reset(bubble);
        }
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
            {
                const float dx{static_cast<float>(x) - bubble.x};
                const float dy{static_cast<float>(y) - bubble.y};
                const float delta{std::fabs(std::sqrt((dx * dx) + (dy * dy)) - bubble.radius)};
                if (delta < edge)
                {
                    const auto brightness{static_cast<uint8_t>(static_cast<float>(bubble.brightness) *
                                                               (1.0F - (delta / edge)))};
                    if (brightness >= 16U)
                    {
                        Display.setPixel(x, y, std::max(brightness, Display.getPixel(x, y)));
                    }
                }
            }
        }
    }
}

#endif // MODE_BUBBLES
