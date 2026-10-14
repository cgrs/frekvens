#pragma once

// Device geometry from the real device headers; accessories, extensions and services are left out.

#if SIM_FREKVENS
#include "config/IkeaFrekvens.h"
#else
#include "config/IkeaObegransad.h"
#endif
