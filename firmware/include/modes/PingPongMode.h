#pragma once

#if MODE_PINGPONG

#include "config/constants.h"      // NOLINT(misc-include-cleaner)
#include "handlers/ClockHandler.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <bits/unique_ptr.h>
#include <deque>

class PingPongMode final : public ModeModule
{
private:
    static constexpr float speed{1e-3F * GRID_COLUMNS};

    float xDec{.0F};
    float yDec{.0F};

    std::deque<uint8_t> paddleA{};
    std::deque<uint8_t> paddleB{};

    uint8_t x{GRID_COLUMNS - 2U};
    uint8_t y{GRID_ROWS / 2U};

    uint16_t deg{135U};

    unsigned long lastMillis{0UL};

    std::unique_ptr<ClockHandler> clock{};

    static constexpr std::array<Option, 1U> declared{
        Option{"clock", "Clock", OptionType::boolean, "table-tennis"},
    };

    bool began{false};

public:
    static constexpr std::string_view name{"Ping-pong"};

    explicit PingPongMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
};

#endif // MODE_PINGPONG
