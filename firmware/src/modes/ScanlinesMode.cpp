#if MODE_SCANLINES

#include "modes/ScanlinesMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 1U, __STRING(MODE_SCANLINES) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 3U, __STRING(MODE_SCANLINES) " is not compatible with this device's display size.");

/**
 * @brief Moves the scanline to the top and clears the display frame.
 */
void ScanlinesMode::begin()
{
    position = 0.0F;
    speed = 0.4F;
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Draws a bright horizontal line with dimmer neighbors, bouncing between the top and bottom rows.
 */
void ScanlinesMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    constexpr float maxY{static_cast<float>(GRID_ROWS - 1U)};
    Display.fillFrame(0U);
    const auto y{static_cast<size_t>(position + 0.5F)};
    if (y < GRID_ROWS)
    {
        Display.drawLineHorizontal(0U, GRID_COLUMNS, y, 220U);
    }
    if (y >= 1U && y - 1U < GRID_ROWS)
    {
        Display.drawLineHorizontal(0U, GRID_COLUMNS, y - 1U, 100U);
    }
    if (y + 1U < GRID_ROWS)
    {
        Display.drawLineHorizontal(0U, GRID_COLUMNS, y + 1U, 100U);
    }
    position += speed;
    if (position > maxY || position < 0.0F)
    {
        speed = -speed;
        position = position < 0.0F ? 0.0F : maxY;
    }
}

#endif // MODE_SCANLINES
