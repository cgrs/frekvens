#pragma once

#if MODE_CLOCK

#include "config/constants.h"     // NOLINT(misc-include-cleaner)
#include "fonts/MediumBoldFont.h" // NOLINT(misc-include-cleaner)
#include "fonts/MediumFont.h"     // NOLINT(misc-include-cleaner)
#include "fonts/MediumWideFont.h" // NOLINT(misc-include-cleaner)
#include "fonts/MiniFont.h"       // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <bits/unique_ptr.h>
#include <vector>

class ClockMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 4U> fontNames{
        MiniFont::name,
        MediumFont::name,
        MediumBoldFont::name,
        MediumWideFont::name,
    };

    static constexpr std::array<Option, 2U> declared{
        Option{"font", "Font", OptionType::choice, "format-font", fontNames},
        Option{"ticking", "Second indicator", OptionType::boolean, "progress-clock"},
    };

    bool ticking{true};

    uint8_t font{0U};

    tm local{};

    bool pending{false};
    bool strikethrough{false};

    int hour{24U};
    int minute{60U};
    int second{60U};

    void drawTicker(uint8_t brightness) const;

public:
    static constexpr std::string_view name{"Clock"};

    explicit ClockMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
};

#endif // MODE_CLOCK
