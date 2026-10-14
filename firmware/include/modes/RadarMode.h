#pragma once

#if MODE_RADAR

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class RadarMode final : public ModeModule
{
private:
    static constexpr uint8_t blipCount{5U};
    static constexpr uint8_t interval{50U};
    static constexpr uint8_t sweepRadius{12U};
    static constexpr float sweepSpeed{0.1F};
    static constexpr int centerX{GRID_COLUMNS / 2};
    static constexpr int centerY{GRID_ROWS / 2};

    struct Blip
    {
        uint8_t x{0U};
        uint8_t y{0U};
        uint8_t age{0U};
        bool active{false};
    };

    float sweepAngle{0.0F};

    std::array<Blip, blipCount> blips{};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Radar"};

    explicit RadarMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_RADAR
