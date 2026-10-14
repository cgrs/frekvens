#pragma once

#if MODE_METABALLS

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class MetaballsMode final : public ModeModule
{
private:
    static constexpr float radius{
        min<float>(static_cast<float>(GRID_COLUMNS * PITCH_HORIZONTAL) / static_cast<float>(PITCH_VERTICAL),
                   static_cast<float>(GRID_ROWS *PITCH_VERTICAL) / static_cast<float>(PITCH_HORIZONTAL)) *
        0.4375F};
    static constexpr float radiusSq{radius * radius};
    static constexpr float speed{static_cast<float>(GRID_COLUMNS) / 640.0F};

    static constexpr uint8_t multiplier{1U << 4U};
    // Summed ball contribution that maps to full brightness, and the gamma applied to the result
    static constexpr float capValue{3.0F};
    static constexpr float gamma{0.7F};
    static constexpr uint8_t interval{50U}; // ms

    struct Ball
    {
        float x;
        float y;
        float xVelocity;
        float yVelocity;
    };

    std::array<Ball, GRID_COLUMNS * GRID_ROWS / 50U> balls{};
    std::array<uint8_t, GRID_COLUMNS * GRID_ROWS> previous{};

    unsigned long lastMillis{0UL};

    static uint8_t toneMap(float value);

public:
    static constexpr std::string_view name{"Metaballs"};

    explicit MetaballsMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_METABALLS
