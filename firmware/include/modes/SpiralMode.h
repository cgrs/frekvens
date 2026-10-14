#pragma once

#if MODE_SPIRAL

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class SpiralMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{30U};
    static constexpr uint8_t points{5U};
    static constexpr float maxRadius{10.0F};
    static constexpr float minRadius{1.0F};
    static constexpr int centerX{GRID_COLUMNS / 2};
    static constexpr int centerY{GRID_ROWS / 2};

    bool expanding{true};

    float angle{0.0F};
    float radius{0.0F};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Spiral"};

    explicit SpiralMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_SPIRAL
