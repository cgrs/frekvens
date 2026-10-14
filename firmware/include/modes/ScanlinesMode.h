#pragma once

#if MODE_SCANLINES

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class ScanlinesMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{60U};

    float position{0.0F};
    float speed{0.4F};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Scanlines"};

    explicit ScanlinesMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_SCANLINES
