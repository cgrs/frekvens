#pragma once

#if MODE_BUBBLES

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class BubblesMode final : public ModeModule
{
private:
    static constexpr uint8_t count{6U};
    static constexpr uint8_t interval{80U};
    static constexpr float edge{0.6F};
    static constexpr float maxRadius{10.0F};

    struct Bubble
    {
        float x{0.0F};
        float y{0.0F};
        float radius{1.0F};
        float speed{0.2F};
        uint8_t brightness{UINT8_MAX};
    };

    std::array<Bubble, count> bubbles{};

    unsigned long lastMillis{0UL};

    static void reset(Bubble &bubble);

public:
    static constexpr std::string_view name{"Bubbles"};

    explicit BubblesMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_BUBBLES
