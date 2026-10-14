#pragma once

#if MODE_MATRIXRAIN

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class MatrixRainMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{50U};
    static constexpr uint8_t fade{15U};
    static constexpr uint8_t maxTrail{8U};

    struct Column
    {
        int16_t y{0};
        uint8_t speed{1U};
        uint8_t length{3U};
        bool active{false};
    };

    std::array<Column, GRID_COLUMNS> columns{};

    unsigned long lastMillis{0UL};

    static void spawn(Column &column, long maxDelay);

public:
    static constexpr std::string_view name{"Matrix rain"};

    explicit MatrixRainMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_MATRIXRAIN
