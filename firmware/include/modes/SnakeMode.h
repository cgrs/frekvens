#pragma once

#if MODE_SNAKE

#include "handlers/ClockHandler.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <bits/unique_ptr.h>
#include <optional>

class SnakeMode final : public ModeModule
{
private:
    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        READY,
        MOVE,
        DEATH,
        REMOVE,
    };

    static constexpr std::array<Option, 1U> declared{
        Option{"clock", "Clock", OptionType::boolean, "snake"},
    };

    bool began{false};

    unsigned long lastMillis{0UL};

    uint8_t blinkCount{0U};

    size_t head{0U};
    size_t length{0U};
    size_t target{0U};

    std::array<size_t, GRID_COLUMNS * GRID_ROWS> snake{};
    std::array<bool, GRID_COLUMNS * GRID_ROWS> occupied{};

    std::unique_ptr<ClockHandler> clock{};

    Stage stage{Stage::READY};

    void blink();
    void clean();
    void idle();
    void move();
    void setDead();
    void setTarget();
    void snakeReset(size_t start);
    void snakeClear();

    [[nodiscard]] bool snakePushBack(size_t pixel);

    [[nodiscard]] size_t snakeAt(size_t index) const;
    [[nodiscard]] size_t snakePopFront();

    [[nodiscard]] std::optional<size_t> findStepAvailable() const;
    [[nodiscard]] std::optional<size_t> findStepPath() const;

public:
    static constexpr std::string_view name{"Snake"};

    explicit SnakeMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
};

#endif // MODE_SNAKE
