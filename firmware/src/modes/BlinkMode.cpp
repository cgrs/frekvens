#if MODE_BLINK

#include "modes/BlinkMode.h"

#include "modules/Trigger.h"
#include "services/DisplayService.h"

/**
 * @brief Advances the blink state when the update interval has elapsed.
 *
 * When microphone support is enabled, the display updates only when the
 * microphone is triggered. Each update alternates between maximum and zero
 * brightness.
 */
void BlinkMode::handle()
{
    if (millis() - lastMillis > (0b1U << 10U) && Trigger::active())
    {
        lastMillis = millis();
        Display.fillFrame(lit ? UINT8_MAX : 0U);
        lit = !lit;
    }
}

#endif // MODE_BLINK
