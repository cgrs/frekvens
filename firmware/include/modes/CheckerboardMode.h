#pragma once

#if MODE_CHECKERBOARD

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class CheckerboardMode final : public ModeModule
{
private:
    static constexpr uint8_t interval{100U};
    static constexpr uint8_t squareSize{2U};
    static constexpr uint8_t step{15U};

    bool inverted{false};

    uint8_t offset{0U};
    uint8_t phase{0U};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Checkerboard"};

    explicit CheckerboardMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_CHECKERBOARD
