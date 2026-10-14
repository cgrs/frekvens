#if MODE_LINES

#include "modes/LinesMode.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 4U, __STRING(MODE_LINES) " is not compatible with this device's display size.");

/**
 * @brief Advances the line animation when its update interval has elapsed.
 *
 * When microphone support is enabled, the animation advances only when the
 * microphone is triggered.
 */
void LinesMode::handle()
{
    if (millis() - lastMillis > INT8_MAX && Trigger::active())
    {
        lastMillis = millis();
        for (uint8_t offset{0U}; offset < GRID_COLUMNS + 2U; offset += 4U)
        {
            if (x + offset < GRID_COLUMNS)
            {
                Display.fillColumn(x + offset, UINT8_MAX);
            }
            if (x + offset >= 2U && x + offset < GRID_COLUMNS + 2U)
            {
                Display.fillColumn(x + offset - 2U, 0U);
            }
        }
        ++x;
        if (x >= 4U)
        {
            x = 0U;
        }
    }
}

#endif // MODE_LINES
