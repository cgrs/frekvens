#if MODE_WEATHER

#include "modes/WeatherMode.h"

#include "fonts/MiniFont.h"                    // NOLINT(misc-include-cleaner)
#include "handlers/BitmapHandler.h"            // NOLINT(misc-include-cleaner)
#include "handlers/TextHandler.h"              // NOLINT(misc-include-cleaner)
#include "services/DisplayService.h" // NOLINT(misc-include-cleaner)

#include <WiFi.h> // NOLINT(misc-include-cleaner)

static_assert(GRID_COLUMNS >= 14U, __STRING(MODE_WEATHER) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 14U, __STRING(MODE_WEATHER) " is not compatible with this device's display size.");

/**
 * @brief Creates the selected provider and schedules an immediate update.
 */
void WeatherMode::begin()
{
    if (!provider)
    {
        makeProvider();
    }
    lastMillis = millis() - provider->interval;
}

/**
 * @brief Updates weather data and renders the current condition and temperature.
 *
 * Refreshes the provider data after the configured interval when Wi-Fi is connected.
 * When both values are available, displays the condition above the centered temperature
 * and publishes the updated state.
 */
void WeatherMode::handle()
{
    if (WiFi.isConnected() && millis() - lastMillis > provider->interval)
    {
        lastMillis = millis();
        provider->update(condition, temperature, lastMillis);
        if (condition.has_value() && temperature.has_value())
        {
            std::visit(
                [&](auto sign)
                {
                    const BitmapHandler bitmap(sign);
                    const MiniFont font;
                    const TextHandler text(std::to_string(temperature.value()) + "°", font);
                    const uint8_t textHeight{text.getHeight()};
                    const uint8_t marginsY{
                        static_cast<uint8_t>(max(0U, GRID_ROWS - bitmap.getHeight() - textHeight) / 3U)};
                    Display.fillFrame(0U);
                    bitmap.draw((GRID_COLUMNS - bitmap.getWidth()) / 2U, marginsY);
                    text.draw((GRID_COLUMNS - text.getWidth()) / 2U, GRID_ROWS - marginsY - textHeight);
                },
                provider->getSign(condition.value()));
            transmitOptions();
        }
    }
}

/**
 * @brief Creates the provider for the selected index.
 */
void WeatherMode::makeProvider()
{
    const std::string_view providerName{providerNames[providerIndex]};
#if WEATHER_OPENMETEO
    if (providerName == OpenMeteoMiddleware::name)
    {
        provider = std::make_unique<OpenMeteoMiddleware>();
    }
#endif // WEATHER_OPENMETEO
#if WEATHER_WTTRIN
    if (providerName == WttrInMiddleware::name)
    {
        provider = std::make_unique<WttrInMiddleware>();
    }
#endif // WEATHER_WTTRIN
#if WEATHER_YR
    if (providerName == YrMiddleware::name)
    {
        provider = std::make_unique<YrMiddleware>();
    }
#endif // WEATHER_YR
}

/**
 * @brief Gets the selected provider.
 *
 * @param index Index into the declared options.
 */
uint8_t WeatherMode::getOption(size_t index) const { return providerIndex; }

/**
 * @brief Selects the provider and discards the readings of the previous one.
 *
 * @param index Index into the declared options.
 * @param value Index into the available providers.
 */
void WeatherMode::setOption(size_t index, uint8_t value)
{
    providerIndex = value;
    makeProvider();
    condition.reset();
    temperature.reset();
    lastMillis = millis() - provider->interval;
}

/**
 * @brief Adds the current condition and temperature to the published state.
 *
 * @param doc Payload to extend.
 */
void WeatherMode::extendState(JsonObject doc) const
{
    if (condition.has_value())
    {
        doc["condition"].set(conditionNames[static_cast<size_t>(condition.value())]);
    }
    if (temperature.has_value())
    {
        doc["temperature"].set(temperature.value());
    }
}

#endif // MODE_WEATHER
