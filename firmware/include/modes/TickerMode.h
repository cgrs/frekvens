#pragma once

#if MODE_TICKER

#include "config/constants.h"     // NOLINT(misc-include-cleaner)
#include "fonts/SmallFont.h"      // NOLINT(misc-include-cleaner)
#include "handlers/TextHandler.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"
#include "services/FontsService.h" // NOLINT(misc-include-cleaner)

#include <array>
#include <bits/unique_ptr.h>

class TickerMode final : public ModeModule
{
private:
    static constexpr uint8_t defaultFont{[]
                                         {
                                             for (size_t idx{0U}; idx < FontsService::names.size(); ++idx)
                                             {
                                                 if (FontsService::names[idx] == SmallFont::name)
                                                 {
                                                     return static_cast<uint8_t>(idx);
                                                 }
                                             }
                                             return static_cast<uint8_t>(0U);
                                         }()};

    static constexpr std::array<Option, 2U> declared{
        Option{"font", "Font", OptionType::choice, "format-font", FontsService::names},
        Option{"message", "Message", OptionType::text, "message", {}, false},
    };

    std::string message{NAME};

    uint8_t fontIndex{defaultFont};

    bool pending{false};

    int8_t offsetY{static_cast<int8_t>(GRID_ROWS / 2U)};

    int16_t offsetX{GRID_COLUMNS};
    int16_t width{0};

    unsigned long lastMillis{0UL};

    std::unique_ptr<const FontModule> font{};

    std::unique_ptr<TextHandler> text{};

public:
    static constexpr std::string_view name{"Ticker"};

    explicit TickerMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
    void end() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
    [[nodiscard]] std::string getText(size_t index) const override;
    [[nodiscard]] bool setText(size_t index, std::string_view value) override;
};

#endif // MODE_TICKER
