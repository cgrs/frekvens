#if MODE_GAMEOFLIFE

#include "modes/GameOfLifeMode.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"

#include <vector>

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_GAMEOFLIFE) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_GAMEOFLIFE) " is not compatible with this device's display size.");

/**
 * @brief Clears the clock area when the clock is enabled.
 */
void GameOfLifeMode::begin()
{
    if (clock != nullptr)
    {
        clock->clear();
    }
}

/**
 * @brief Updates the optional clock and advances the Conway's Game of Life simulation.
 */
void GameOfLifeMode::handle()
{
    if (clock != nullptr)
    {
        clock->handle();
    }
    if (millis() - lastMillis <= UINT8_MAX || !Trigger::active())
    {
        return;
    }
    std::vector<bool> seeds(GRID_COLUMNS * (GRID_ROWS - yMin), false);
    for (uint8_t idx{active}; idx < static_cast<uint8_t>(GRID_COLUMNS * (GRID_ROWS - yMin) / (0b1U << 4U)); ++idx)
    {
        seeds[random(1, GRID_COLUMNS - 1) + (random(yMin + 1U, GRID_ROWS - 1) * (GRID_COLUMNS - yMin))] = true;
    }
    lastMillis = millis();
    active = 0U;
    for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
    {
        for (uint8_t y{yMin}; y < GRID_ROWS; ++y)
        {
            uint8_t count{0U};
            for (uint8_t _x{static_cast<uint8_t>(max<int16_t>(x - 1, 0))}; _x <= x + 1U && _x < GRID_COLUMNS; ++_x)
            {
                for (uint8_t _y{static_cast<uint8_t>(max<int16_t>(yMin, y - 1U))}; _y <= y + 1U && _y < GRID_ROWS; ++_y)
                {
                    if ((_x != x || _y != y) &&
                        (seeds[_x + (_y * (GRID_COLUMNS - yMin))] || Display.getPixel(_x, _y) != 0U))
                    {
                        ++count;
                    }
                }
            }
            const bool lit{seeds[x + (y * (GRID_COLUMNS - yMin))] || Display.getPixel(x, y) != 0U};
            if (lit && (count < 2U || count > 3U))
            {
                Display.setPixel(x, y, 0U);
            }
            else if (!lit && count == 3U)
            {
                Display.setPixel(x, y, brightness);
                ++active;
            }
        }
    }
}

/**
 * @brief Gets the value of an option.
 *
 * @param index Index into the declared options.
 * @return Whether the clock is enabled.
 */
uint8_t GameOfLifeMode::getOption(size_t index) const { return static_cast<uint8_t>(clock != nullptr); }

/**
 * @brief Enables or disables the clock and adjusts the playfield to match.
 *
 * @param index Index into the declared options.
 * @param value Whether the clock should be enabled.
 */
void GameOfLifeMode::setOption(size_t index, uint8_t value)
{
    if (value != 0U)
    {
        yMin = 5U;
        clock = std::make_unique<ClockHandler>();
        clock->clear();
        brightness = INT8_MAX;
    }
    else
    {
        clock.reset();
        yMin = 0U;
        brightness = UINT8_MAX;
    }
}

#endif // MODE_GAMEOFLIFE
