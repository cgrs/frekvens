#if MODE_MATRIXRAIN

#include "modes/MatrixRainMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 1U, __STRING(MODE_MATRIXRAIN) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_MATRIXRAIN) " is not compatible with this device's display size.");

/**
 * @brief Starts a column above the display with a random speed and trail length.
 *
 * @param column Column to start.
 * @param maxDelay Upper bound of how many rows above the display the head starts.
 */
void MatrixRainMode::spawn(Column &column, long maxDelay)
{
    column.y = static_cast<int16_t>(-random(maxDelay));
    column.speed = static_cast<uint8_t>(random(1, 4));
    column.length = static_cast<uint8_t>(random(3, maxTrail));
    column.active = true;
}

/**
 * @brief Starts most columns at random heights and clears the display frame.
 */
void MatrixRainMode::begin()
{
    for (Column &column : columns)
    {
        spawn(column, GRID_ROWS);
        column.active = random(100) > 30;
    }
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Fades the frame, then draws and advances every active column.
 *
 * Idle columns have a small chance of restarting each frame.
 */
void MatrixRainMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    for (size_t idx{0U}; idx < GRID_COLUMNS * GRID_ROWS; ++idx)
    {
        const uint8_t brightness{Display.getPixel(idx)};
        Display.setPixel(idx, brightness > fade ? static_cast<uint8_t>(brightness - fade) : 0U);
    }
    for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
    {
        Column &column{columns[x]};
        if (!column.active)
        {
            if (random(100) > 95)
            {
                spawn(column, 5);
            }
            continue;
        }
        if (column.y >= 0 && column.y < static_cast<int16_t>(GRID_ROWS))
        {
            Display.setPixel(x, static_cast<uint8_t>(column.y), UINT8_MAX);
        }
        for (uint8_t j{1U}; j < column.length; ++j)
        {
            const int16_t trailY{static_cast<int16_t>(column.y - j)};
            if (trailY >= 0 && trailY < static_cast<int16_t>(GRID_ROWS))
            {
                Display.setPixel(x, static_cast<uint8_t>(trailY),
                                 static_cast<uint8_t>(UINT8_MAX - (j * UINT8_MAX / column.length)));
            }
        }
        column.y = static_cast<int16_t>(column.y + column.speed);
        if (column.y - column.length >= static_cast<int16_t>(GRID_ROWS))
        {
            column.active = false;
        }
    }
}

#endif // MODE_MATRIXRAIN
