#include "Arduino.h"

#include <random>

static unsigned long clockMs{0UL};
static std::mt19937 engine{1U};

unsigned long millis() { return clockMs; }
void simAdvance(unsigned long ms) { clockMs += ms; }

long random(long max) { return max <= 0 ? 0 : random(0, max); }
long random(long min, long max)
{
    return max <= min ? min : std::uniform_int_distribution<long>(min, max - 1)(engine);
}
void randomSeed(unsigned long seed) { engine.seed(static_cast<std::mt19937::result_type>(seed)); }

// Fixed start (2026-01-01 12:00:00 UTC) plus virtual time, so runs and frames are reproducible.
bool getLocalTime(struct tm *info, uint32_t)
{
    const time_t now{1767268800 + static_cast<time_t>(clockMs / 1000UL)};
    return gmtime_r(&now, info) != nullptr;
}
