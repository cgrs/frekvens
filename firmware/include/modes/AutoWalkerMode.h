#pragma once

#if MODE_AUTOWALKER

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class AutoWalkerMode final : public ModeModule
{
private:
    static constexpr size_t cells{GRID_COLUMNS * GRID_ROWS};
    static constexpr unsigned long stepInterval{70UL};
    static constexpr unsigned long holdDuration{1200UL};
    static constexpr unsigned long fadeInterval{30UL};
    static constexpr uint8_t fadeStep{17U};
    static constexpr uint8_t wanderOneIn{5U};
    static constexpr uint8_t brightnessWalker{UINT8_MAX};
    static constexpr uint8_t brightnessVisit1{20U};
    static constexpr uint8_t brightnessVisit2{90U};
    static constexpr uint8_t brightnessVisitMax{200U};

    enum class Stage : uint8_t
    {
        RUN,
        HOLD,
        FADE,
    };

    Stage stage{Stage::RUN};

    std::array<std::array<uint8_t, GRID_COLUMNS>, GRID_ROWS> counts{};
    std::array<std::array<uint8_t, GRID_COLUMNS>, GRID_ROWS> fade{};

    size_t visited{0U};

    uint8_t x{0U};
    uint8_t y{0U};

    unsigned long lastMillis{0UL};

    [[nodiscard]] static uint8_t brightnessFor(uint8_t count);

    void restart();
    void step();
    void startHold();
    void fadeOut();
    void render() const;

public:
    static constexpr std::string_view name{"Auto walker"};

    explicit AutoWalkerMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_AUTOWALKER
