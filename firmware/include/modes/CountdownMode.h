#pragma once

#if MODE_COUNTDOWN

#include "config/constants.h"     // NOLINT(misc-include-cleaner)
#include "fonts/MediumBoldFont.h" // NOLINT(misc-include-cleaner)
#include "fonts/MediumFont.h"     // NOLINT(misc-include-cleaner)
#include "fonts/MediumWideFont.h" // NOLINT(misc-include-cleaner)
#include "fonts/MiniFont.h"       // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <chrono>

class CountdownMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 4U> fontNames{
        MiniFont::name,
        MediumFont::name,
        MediumBoldFont::name,
        MediumWideFont::name,
    };

    static inline std::chrono::time_point<std::chrono::system_clock> epoch{};

    static constexpr std::array<Option, 1U> declared{
        Option{"font", "Font", OptionType::choice, "format-font", fontNames},
    };

    uint8_t font{0U};

    bool odd{false};

    uint8_t blink{0U};
    uint8_t lower{0U};
    uint8_t upper{0U};

    void save();

public:
    static constexpr std::string_view name{"Countdown"};

    explicit CountdownMode() : ModeModule(name) {};

    void configure() override;
    void begin() override;
    void handle() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
    void extendState(JsonObject doc) const override;

    void onReceive(JsonObjectConst payload, std::string_view source) override;

#if EXTENSION_HOMEASSISTANT
    void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique) override;
#endif
};

#endif // MODE_COUNTDOWN
