#if MODE_SPARKLEFIELD

#include "modes/SparkleFieldMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 1U, __STRING(MODE_SPARKLEFIELD) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 1U, __STRING(MODE_SPARKLEFIELD) " is not compatible with this device's display size.");

/**
 * @brief Clears all sparkles and the display frame.
 */
void SparkleFieldMode::begin()
{
    sparkles.fill(0U);
    lastMillis = millis();
    Display.fillFrame(0U);
}

/**
 * @brief Fades existing sparkles, ignites new ones at random positions and redraws the frame.
 */
void SparkleFieldMode::handle()
{
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    for (uint8_t &value : sparkles)
    {
        value = value > fade ? static_cast<uint8_t>(value - fade) : 0U;
    }
    for (uint8_t i{0U}; i < spawn; ++i)
    {
        sparkles[static_cast<size_t>(random(GRID_COLUMNS * GRID_ROWS))] = UINT8_MAX;
    }
    for (size_t idx{0U}; idx < sparkles.size(); ++idx)
    {
        Display.setPixel(idx, sparkles[idx]);
    }
}

#endif // MODE_SPARKLEFIELD
