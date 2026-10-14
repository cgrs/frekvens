#include "modules/Trigger.h"

#include "services/ExtensionsService.h" // NOLINT(misc-include-cleaner)

/**
 * @brief Reports whether modes should react.
 *
 * @return The microphone's trigger state, or true when the microphone extension is not built.
 */
bool Trigger::active()
{
#if EXTENSION_MICROPHONE
    return Extensions.Microphone().isTriggered();
#else
    return true;
#endif // EXTENSION_MICROPHONE
}
