#pragma once

#if MODE_METEORSHOWER

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class MeteorShowerMode final : public ModeModule
{
private:
    static constexpr uint8_t count{6U};
    static constexpr uint8_t interval{50U};
    static constexpr uint8_t tail{4U};

    struct Meteor
    {
        float x{0.0F};
        float y{0.0F};
        float vx{0.4F};
        float vy{0.9F};
    };

    std::array<Meteor, count> meteors{};

    unsigned long lastMillis{0UL};

    static void reset(Meteor &meteor);

public:
    static constexpr std::string_view name{"Meteor shower"};

    explicit MeteorShowerMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_METEORSHOWER
