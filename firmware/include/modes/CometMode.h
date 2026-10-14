#pragma once

#if MODE_COMET

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class CometMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{40U};
    static constexpr uint8_t fade{14U};
    static constexpr uint8_t tail{3U};

    float x{0.0F};
    float y{0.0F};
    float vx{0.8F};
    float vy{0.5F};

    std::array<uint8_t, GRID_COLUMNS * GRID_ROWS> trail{};

    unsigned long lastMillis{0UL};

    void reset();

public:
    static constexpr std::string_view name{"Comet"};

    explicit CometMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_COMET
