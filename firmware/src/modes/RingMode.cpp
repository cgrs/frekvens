#if MODE_RING

#include "modes/RingMode.h"

#include "handlers/BitmapHandler.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"

static_assert(GRID_COLUMNS == 16U, __STRING(MODE_RING) " is not compatible with this device's display size.");
static_assert(GRID_ROWS == 16U, __STRING(MODE_RING) " is not compatible with this device's display size.");

/**
 * @brief Updates and displays the ring animation when an update is due.
 *
 * With microphone support enabled, an update also requires the microphone to
 * be triggered. Each update redraws the current ring frame and reverses the
 * animation direction at either end of the ring sequence.
 */
void RingMode::handle()
{
    if (millis() - lastMillis > INT8_MAX && Trigger::active())
    {
        lastMillis = millis();

        Display.fillFrame(0U);
        BitmapHandler(ring[index]).draw();
        direction ? index++ : index--;
        if (index == 0U || index == ring.size() - 1U)
        {
            direction = !direction;
        }
    }
}

#endif // MODE_RING
