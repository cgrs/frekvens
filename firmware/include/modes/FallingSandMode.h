#pragma once

#if MODE_FALLINGSAND

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class FallingSandMode final : public ModeModule
{
private:
    static constexpr uint8_t spawnColumnMin{1U};
    static constexpr uint8_t spawnColumnMax{GRID_COLUMNS - 2U};
    static constexpr uint8_t spawnColumnStart{GRID_COLUMNS / 2U - 1U};
    static constexpr unsigned long tickInterval{50UL};
    static constexpr unsigned long spawnInterval{180UL};
    static constexpr unsigned long drainInterval{80UL};
    static constexpr unsigned long wanderMin{600UL};
    static constexpr unsigned long wanderMax{1200UL};

    enum class Stage : uint8_t
    {
        SPAWN,
        DRAIN,
    };

    Stage stage{Stage::SPAWN};

    std::array<std::array<uint8_t, GRID_COLUMNS>, GRID_ROWS> amount{};

    uint8_t spawnColumn{spawnColumnStart};

    unsigned long tickMillis{0UL};
    unsigned long spawnMillis{0UL};
    unsigned long drainMillis{0UL};
    unsigned long wanderMillis{0UL};
    unsigned long wanderDelay{wanderMin};

    [[nodiscard]] bool topRowOccupied() const;
    [[nodiscard]] bool empty() const;

    void wander();
    void spawn();
    void physics();
    void drain();
    void render() const;

public:
    static constexpr std::string_view name{"Falling sand"};

    explicit FallingSandMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_FALLINGSAND
