#if MODE_GLITTER

#include "modes/GlitterMode.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"

/**
 * @brief Sets a randomly selected display pixel to a random brightness when activated.
 */
void GlitterMode::handle()
{
    if (Trigger::active())
    {
        Display.setPixel(random(GRID_COLUMNS * GRID_ROWS), static_cast<uint8_t>(random(1, 0b1U << 8U)));
    }
}

#endif // MODE_GLITTER
