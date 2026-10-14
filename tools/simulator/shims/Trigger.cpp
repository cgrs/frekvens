#include "modules/Trigger.h"

// The simulator has no input source; modes always see an active trigger.
bool Trigger::active() { return true; }
