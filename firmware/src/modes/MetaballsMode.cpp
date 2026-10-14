#if MODE_METABALLS

#include "modes/MetaballsMode.h"

#include "modules/Trigger.h"
#include "services/DisplayService.h" // NOLINT(misc-include-cleaner)

#include <algorithm>
#include <cmath>

static_assert(GRID_COLUMNS * GRID_ROWS >= 50U,
              __STRING(MODE_METABALLS) " is not compatible with this device's display size.");

/**
 * @brief Maps the summed ball contribution to a gamma-corrected brightness.
 *
 * @param value Summed contribution of all balls at a pixel.
 * @return Brightness, where `capValue` and above is full brightness.
 */
uint8_t MetaballsMode::toneMap(float value)
{
    return static_cast<uint8_t>(powf(std::min(value, capValue) / capValue, 1.0F / gamma) * static_cast<float>(UINT8_MAX));
}

/**
 * @brief Clears the display and gives every ball a random position and velocity.
 */
void MetaballsMode::begin()
{
    Display.fillFrame(0U);
    previous.fill(0U);
    for (Ball &ball : balls)
    {
        ball.x = static_cast<float>(random(GRID_COLUMNS));
        ball.y = static_cast<float>(random(GRID_ROWS));
        ball.xVelocity = speed * static_cast<float>(random(1, multiplier) * ((random(2) * 2) - 1));
        ball.yVelocity = speed * static_cast<float>(random(1, multiplier) * ((random(2) * 2) - 1));
    }
    lastMillis = millis();
}

/**
 * @brief Renders every pixel from the summed ball contributions, then moves the balls.
 *
 * Runs every `interval` ms; only pixels whose brightness changed are written.
 */
void MetaballsMode::handle()
{
    if (!Trigger::active())
    {
        return;
    }
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
#if PITCH_HORIZONTAL != PITCH_VERTICAL
    const bool rotated{(static_cast<uint8_t>(Display.getOrientation()) & 0b1U) != 0U};
    const float xRatio{static_cast<float>(2U * (rotated ? PITCH_VERTICAL : PITCH_HORIZONTAL)) /
                       static_cast<float>(PITCH_VERTICAL + PITCH_HORIZONTAL)};
    const float yRatio{static_cast<float>(2U * (rotated ? PITCH_HORIZONTAL : PITCH_VERTICAL)) /
                       static_cast<float>(PITCH_VERTICAL + PITCH_HORIZONTAL)};
#endif // PITCH_HORIZONTAL != PITCH_VERTICAL
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            float value{0.0F};
            for (const Ball &ball : balls)
            {
#if PITCH_HORIZONTAL == PITCH_VERTICAL
                const float xDistance{ball.x - static_cast<float>(x)};
                const float yDistance{ball.y - static_cast<float>(y)};
#else
                const float xDistance{(ball.x - static_cast<float>(x)) * xRatio};
                const float yDistance{(ball.y - static_cast<float>(y)) * yRatio};
#endif // PITCH_HORIZONTAL == PITCH_VERTICAL
                const float distanceSq{(xDistance * xDistance) + (yDistance * yDistance)};
                if (distanceSq < radiusSq)
                {
                    const float falloff{1.0F - (distanceSq / radiusSq)};
                    value += falloff * falloff;
                }
            }
            const uint8_t brightness{toneMap(value)};
            uint8_t &last{previous[(static_cast<size_t>(y) * GRID_COLUMNS) + x]};
            if (last != brightness)
            {
                Display.setPixel(x, y, brightness);
                last = brightness;
            }
        }
    }
    for (Ball &ball : balls)
    {
        ball.x += ball.xVelocity;
        ball.y += ball.yVelocity;
        if (ball.x < .0F)
        {
            ball.x = .0F;
            ball.xVelocity = speed * static_cast<float>(random(1, multiplier));
        }
        else if (ball.x > GRID_COLUMNS - 1U)
        {
            ball.x = GRID_COLUMNS - 1U;
            ball.xVelocity = -speed * static_cast<float>(random(1, multiplier));
        }
        if (ball.y < .0F)
        {
            ball.y = .0F;
            ball.yVelocity = speed * static_cast<float>(random(1, multiplier));
        }
        else if (ball.y > GRID_ROWS - 1U)
        {
            ball.y = GRID_ROWS - 1U;
            ball.yVelocity = -speed * static_cast<float>(random(1, multiplier));
        }
    }
}

#endif // MODE_METABALLS
