#pragma once

#if MODE_WAVE

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class WaveMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{50U};
    static constexpr float speed{0.2F};

    float phase{0.0F};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Wave"};

    explicit WaveMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_WAVE
