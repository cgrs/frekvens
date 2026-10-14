#pragma once

#if MODE_GAMEOFLIFE

#include "handlers/ClockHandler.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <bits/unique_ptr.h>

class GameOfLifeMode final : public ModeModule
{
private:
    uint8_t active{0U};
    uint8_t brightness{UINT8_MAX};
    uint8_t yMin{0U};

    unsigned long lastMillis{0UL};

    std::unique_ptr<ClockHandler> clock{};

    static constexpr std::array<Option, 1U> declared{
        Option{"clock", "Clock", OptionType::boolean, "one-up"},
    };

public:
    static constexpr std::string_view name{"Game of Life"};

    explicit GameOfLifeMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
};

#endif // MODE_GAMEOFLIFE
