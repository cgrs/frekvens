#pragma once

#if MODE_SPARKLEFIELD

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class SparkleFieldMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{70U};
    static constexpr uint8_t fade{18U};
    static constexpr uint8_t spawn{6U};

    std::array<uint8_t, GRID_COLUMNS * GRID_ROWS> sparkles{};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Sparkle field"};

    explicit SparkleFieldMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_SPARKLEFIELD
