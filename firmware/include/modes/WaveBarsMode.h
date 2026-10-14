#pragma once

#if MODE_WAVEBARS

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class WaveBarsMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{60U};

    float phase{0.0F};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Wave bars"};

    explicit WaveBarsMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_WAVEBARS
