#if MODE_METEORSHOWER

#include "modes/MeteorShowerMode.h"

#include "services/DisplayService.h"

#include <algorithm>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_METEORSHOWER) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_METEORSHOWER) " is not compatible with this device's display size.");

/**
 * @brief Places a meteor above the display with a random column and diagonal velocity.
 *
 * @param meteor Meteor to reset.
 */
void MeteorShowerMode::reset(Meteor &meteor)
{
    meteor.x = static_cast<float>(random(GRID_COLUMNS));
    meteor.y = static_cast<float>(random(-12, 0));
    meteor.vx = 0.4F + (static_cast<float>(random(4)) / 10.0F);
    meteor.vy = 0.9F + (static_cast<float>(random(6)) / 10.0F);
}

/**
 * @brief Launches all meteors and clears the display frame.
 */
void MeteorShowerMode::begin()
{
    for (Meteor &meteor : meteors)
    {
        reset(meteor);
    }
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Moves each meteor diagonally and redraws the frame with a fading tail.
 *
 * Meteors that leave the display are relaunched from above.
 */
void MeteorShowerMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    Display.fillFrame(0U);
    for (Meteor &meteor : meteors)
    {
        meteor.x += meteor.vx;
        meteor.y += meteor.vy;
        if (meteor.x > static_cast<float>(GRID_COLUMNS + 2U) || meteor.y > static_cast<float>(GRID_ROWS + 2U))
        {
            reset(meteor);
        }
        for (uint8_t t{0U}; t < tail; ++t)
        {
            const float tx{meteor.x - (meteor.vx * static_cast<float>(t)) + 0.5F};
            const float ty{meteor.y - (meteor.vy * static_cast<float>(t)) + 0.5F};
            if (tx >= 0.0F && ty >= 0.0F && static_cast<size_t>(tx) < GRID_COLUMNS &&
                static_cast<size_t>(ty) < GRID_ROWS)
            {
                const auto x{static_cast<uint8_t>(tx)};
                const auto y{static_cast<uint8_t>(ty)};
                Display.setPixel(x, y, std::max(static_cast<uint8_t>(230U - (t * 50U)), Display.getPixel(x, y)));
            }
        }
    }
}

#endif // MODE_METEORSHOWER
