#pragma once

// Host stand-in for the Arduino core, force-included into every translation unit.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#ifndef __STRING
#define __STRING(x) #x
#endif
#define DRAM_ATTR
#define IRAM_ATTR

using std::max;
using std::min;

constexpr float PI{3.14159265358979323846F};
constexpr float TWO_PI{2.0F * PI};
constexpr float HALF_PI{PI / 2.0F};
constexpr float DEG_TO_RAD{PI / 180.0F};
constexpr float RAD_TO_DEG{180.0F / PI};

/** Virtual clock in ms, advanced by the simulator. */
unsigned long millis();
void simAdvance(unsigned long ms);

long random(long max);
long random(long min, long max);
void randomSeed(unsigned long seed);

bool getLocalTime(struct tm *info, uint32_t ms = 5000);

template <typename T, typename L, typename H> constexpr auto constrain(T value, L low, H high)
{
    return value < low ? low : (value > high ? high : value);
}

// ESP-IDF logging is dropped.
#define ESP_LOGV(tag, ...) ((void)0)
#define ESP_LOGD(tag, ...) ((void)0)
#define ESP_LOGI(tag, ...) ((void)0)
#define ESP_LOGW(tag, ...) ((void)0)
#define ESP_LOGE(tag, ...) ((void)0)
