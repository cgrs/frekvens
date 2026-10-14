#if MODE_COMET

#include "modes/CometMode.h"

#include "services/DisplayService.h"

#include <algorithm>

static_assert(GRID_COLUMNS >= 4U, __STRING(MODE_COMET) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 4U, __STRING(MODE_COMET) " is not compatible with this device's display size.");

/**
 * @brief Places the comet at a random position, heading in a random diagonal direction.
 */
void CometMode::reset()
{
    x = static_cast<float>(random(GRID_COLUMNS));
    y = static_cast<float>(random(GRID_ROWS));
    vx = random(2) == 0 ? 0.8F : -0.8F;
    vy = random(2) == 0 ? 0.5F : -0.5F;
}

/**
 * @brief Clears the trail and launches a new comet.
 */
void CometMode::begin()
{
    trail.fill(0U);
    reset();
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Fades the trail, moves the comet, bouncing it off the display edges, and redraws the frame.
 */
void CometMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    for (uint8_t &value : trail)
    {
        value = value > fade ? static_cast<uint8_t>(value - fade) : 0U;
    }
    constexpr float maxX{static_cast<float>(GRID_COLUMNS - 1U)};
    constexpr float maxY{static_cast<float>(GRID_ROWS - 1U)};
    x += vx;
    y += vy;
    if (x < 0.0F || x > maxX)
    {
        vx = -vx;
        x = x < 0.0F ? 0.0F : maxX;
    }
    if (y < 0.0F || y > maxY)
    {
        vy = -vy;
        y = y < 0.0F ? 0.0F : maxY;
    }
    const auto headX{static_cast<size_t>(x + 0.5F)};
    const auto headY{static_cast<size_t>(y + 0.5F)};
    if (headX < GRID_COLUMNS && headY < GRID_ROWS)
    {
        trail[headX + (headY * GRID_COLUMNS)] = UINT8_MAX;
    }
    for (uint8_t i{1U}; i <= tail; ++i)
    {
        const float tx{x - (vx * static_cast<float>(i)) + 0.5F};
        const float ty{y - (vy * static_cast<float>(i)) + 0.5F};
        if (tx >= 0.0F && ty >= 0.0F && static_cast<size_t>(tx) < GRID_COLUMNS && static_cast<size_t>(ty) < GRID_ROWS)
        {
            uint8_t &value{trail[static_cast<size_t>(tx) + (static_cast<size_t>(ty) * GRID_COLUMNS)]};
            value = std::max(value, static_cast<uint8_t>(220U - (i * 50U)));
        }
    }
    for (size_t idx{0U}; idx < trail.size(); ++idx)
    {
        Display.setPixel(idx, trail[idx]);
    }
}

#endif // MODE_COMET
