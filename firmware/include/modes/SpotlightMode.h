#pragma once

#if MODE_SPOTLIGHT

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class SpotlightMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{45U};
    static constexpr float radius{4.5F};
    static constexpr uint8_t margin{4U};

    float x{0.0F};
    float y{0.0F};
    float vx{0.3F};
    float vy{0.2F};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Spotlight"};

    explicit SpotlightMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_SPOTLIGHT
