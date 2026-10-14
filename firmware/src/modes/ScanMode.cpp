#if MODE_SCAN

#include "modes/ScanMode.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 3U, __STRING(MODE_SCAN) " is not compatible with this device's display size.");

/**
 * @brief Advances the scan highlight across the display.
 *
 * Updates the highlight after at least INT8_MAX milliseconds have elapsed.
 * When microphone support is enabled, a microphone trigger is also required.
 * Clears the column two positions behind the highlight and wraps the scan
 * position after GRID_COLUMNS + 2U.
 */
void ScanMode::handle()
{
    if (millis() - lastMillis > INT8_MAX && Trigger::active())
    {
        lastMillis = millis();
        if (x < GRID_COLUMNS)
        {
            Display.fillColumn(x, UINT8_MAX);
        }
        if (x >= 2U)
        {
            Display.fillColumn(x - 2U, 0U);
        }
        ++x;
        if (x >= GRID_COLUMNS + 2U)
        {
            x = 0U;
        }
    }
}

#endif // MODE_SCAN
