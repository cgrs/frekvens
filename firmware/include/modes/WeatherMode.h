#pragma once

#if MODE_WEATHER

#include "config/constants.h"                // NOLINT(misc-include-cleaner)
#include "middlewares/OpenMeteoMiddleware.h" // NOLINT(misc-include-cleaner)
#include "middlewares/WttrInMiddleware.h"    // NOLINT(misc-include-cleaner)
#include "middlewares/YrMiddleware.h"        // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

class WeatherMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 9U> conditionNames{
        "Clear",
        "Cloudy",
        "Partly cloudy",
        "Exception",
        "Fog",
        "Rain",
        "Snow",
        "Thunder",
        "Wind",
    };

    static constexpr auto providerNames{std::to_array<std::string_view>({
#if WEATHER_OPENMETEO
        OpenMeteoMiddleware::name,
#endif
#if WEATHER_WTTRIN
        WttrInMiddleware::name,
#endif
#if WEATHER_YR
        YrMiddleware::name,
#endif
    })};

    static constexpr std::array<Option, 1U> declared{
        Option{"provider", "Provider", OptionType::choice, "weather-partly-cloudy", providerNames},
    };

    uint8_t providerIndex{0U};

    unsigned long lastMillis{0UL};

    std::optional<WeatherHandler::Condition> condition{};

    std::optional<int16_t> temperature{};

    std::unique_ptr<WeatherHandler> provider{};

    void makeProvider();

public:
    static constexpr std::string_view name{"Weather"};

    explicit WeatherMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
    void extendState(JsonObject doc) const override;
};

#endif // MODE_WEATHER
