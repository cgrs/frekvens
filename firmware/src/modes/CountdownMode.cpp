#if MODE_COUNTDOWN

#include "modes/CountdownMode.h"

#include "extensions/HomeAssistantExtension.h" // NOLINT(misc-include-cleaner)
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/FontsService.h"

#include <array>
#include <iomanip>
#include <nvs.h>
#include <sstream>
#include <string_view>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_COUNTDOWN) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 11U, __STRING(MODE_COUNTDOWN) " is not compatible with this device's display size.");

/**
 * @brief Loads the persisted countdown target and publishes the current settings.
 */
void CountdownMode::configure()
{
    nvs_handle_t handle{};
    if (nvs_open(name.data(), nvs_open_mode_t::NVS_READONLY, &handle) == ESP_OK)
    {
        int64_t _epoch{0};
        if (nvs_get_i64(handle, "epoch", &_epoch) == ESP_OK && _epoch != 0)
        {
            epoch = std::chrono::system_clock::time_point{std::chrono::seconds{_epoch}};
        }
        nvs_close(handle);
    }
    ModeModule::configure();
}

/**
 * @brief Resets the countdown display state and blinking.
 */
void CountdownMode::begin()
{
    blink = 0U;
    lower = 0U;
    upper = 0U;
}

/**
 * @brief Updates the countdown display and handles completion blinking.
 *
 * Redraws the remaining hours and minutes or minutes and seconds when they
 * change. When the countdown reaches zero, begins blinking the display and
 * publishes a completion event.
 */
void CountdownMode::handle()
{
    const std::chrono::nanoseconds _nanoseconds{epoch - std::chrono::system_clock::now()};
    const std::chrono::hours _hours{std::chrono::duration_cast<std::chrono::hours>(_nanoseconds)};
    const std::chrono::minutes _minutes{std::chrono::duration_cast<std::chrono::minutes>(_nanoseconds - _hours)};
    const int64_t hours{_hours.count()};
    const int64_t minutes{_minutes.count()};
    const int64_t seconds{std::chrono::duration_cast<std::chrono::seconds>(_nanoseconds - _hours - _minutes).count()};
    const uint8_t _upper{static_cast<uint8_t>(std::clamp<int64_t>(hours > 0 ? hours % 100 : minutes, 0, 99))};
    const uint8_t _lower{static_cast<uint8_t>(std::clamp<int64_t>(hours > 0 ? minutes : seconds, 0, 99))};
    if (_lower != lower || _upper != upper)
    {
        upper = _upper;
        lower = _lower;
        if (seconds >= 0 && minutes >= 0 && hours >= 0)
        {
            const std::unique_ptr<const FontModule> fontModule{Fonts.get(fontNames[font])};
            const TextHandler _tl(std::to_string(_upper / 10U), *fontModule);
            const TextHandler _tr(std::to_string(_upper % 10U), *fontModule);
            const TextHandler _bl(std::to_string(lower / 10U), *fontModule);
            const TextHandler _br(std::to_string(lower % 10U), *fontModule);
            const uint8_t fontWidth{max({_tl.getWidth(), _tr.getWidth(), _bl.getWidth(), _br.getWidth()})};
            Display.fillFrame(0U);
            _tl.draw((GRID_COLUMNS / 2U) - 1U - fontWidth + ((fontWidth - _tl.getWidth()) / 2U),
                     static_cast<int8_t>((GRID_ROWS / 2U) - 1U - _tl.getHeight()));
            _tr.draw((GRID_COLUMNS / 2U) + 1U + ((fontWidth - _tr.getWidth()) / 2U),
                     static_cast<int8_t>((GRID_ROWS / 2U) - 1U - _tr.getHeight()));
            _bl.draw((GRID_COLUMNS / 2U) - 1U - fontWidth + ((fontWidth - _bl.getWidth()) / 2U),
                     static_cast<int8_t>((GRID_COLUMNS / 2U) + static_cast<int8_t>(_bl.getHeight() > 5U)));
            _br.draw((GRID_COLUMNS / 2U) + 1U + ((fontWidth - _br.getWidth()) / 2U),
                     static_cast<int8_t>((GRID_COLUMNS / 2U) + static_cast<int8_t>(_br.getHeight() > 5U)));
            if (seconds == 0 && minutes == 0 && hours == 0)
            {
                blink = INT8_MAX;
                odd = true;
                JsonDocument doc{};
                doc["event"].set("done");
                Device.transmit(doc.as<JsonObjectConst>(), name, false);
            }
        }
    }
    else if (blink != 0U && odd == static_cast<bool>(static_cast<uint64_t>(seconds) & 0b1U))
    {
        --blink;
        odd = !odd;
        Display.invertFrame();
    }
}

void CountdownMode::save()
{
    blink = 0U;
    nvs_handle_t handle{};
    if (nvs_open(name.data(), nvs_open_mode_t::NVS_READWRITE, &handle) == ESP_OK)
    {
        nvs_set_i64(
            handle, "epoch", std::chrono::duration_cast<std::chrono::seconds>(epoch.time_since_epoch()).count());
        nvs_commit(handle);
        nvs_close(handle);
    }
    transmitOptions();
}

/**
 * @brief Gets the selected font.
 *
 * @param index Index into the declared options.
 */
uint8_t CountdownMode::getOption(size_t index) const { return font; }

/**
 * @brief Selects the font; the next frame redraws with it.
 *
 * @param index Index into the declared options.
 * @param value Index into the available fonts.
 */
void CountdownMode::setOption(size_t index, uint8_t value)
{
    font = value;
    upper = lower = 0xFFU;
}

/**
 * @brief Adds the countdown timestamp to the published state.
 *
 * The timestamp is formatted as a local ISO-like date and time.
 *
 * @param doc Payload to extend.
 */
void CountdownMode::extendState(JsonObject doc) const
{
    std::array<char, 32U> buffer{};
    time_t timer{std::chrono::system_clock::to_time_t(epoch)};
    tm local{*std::localtime(&timer)};
    const size_t length{strftime(buffer.data(), buffer.size(), "%FT%T", &local)};
    doc["timestamp"].set(std::string_view(buffer.data(), min(buffer.size(), length)));
}

/**
 * @brief Applies the font and updates the target time from a received payload.
 *
 * @param payload Payload containing a font name, a duration in seconds, or a local timestamp.
 * @param source Source identifier for the received payload.
 */
void CountdownMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    ModeModule::onReceive(payload, source);
    // Time or timestamp
    if (payload["time"].is<uint32_t>())
    {
        epoch = std::chrono::system_clock::now() + std::chrono::seconds(payload["time"].as<uint32_t>());
        save();
    }
    else if (payload["timestamp"].is<const char *>())
    {
        tm local{};
        strptime(payload["timestamp"].as<const char *>(), "%FT%T", &local);
        local.tm_isdst = -1;
        epoch = std::chrono::system_clock::from_time_t(mktime(&local));
        save();
    }
}

#if EXTENSION_HOMEASSISTANT
/**
 * @brief Adds the font selector and a datetime entity for the countdown target.
 *
 * @param discovery Home Assistant discovery document to populate.
 * @param topic Base command and state topic for the countdown mode.
 * @param unique Unique identifier prefix for the discovered entities.
 */
void CountdownMode::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    ModeModule::onHomeAssistant(discovery, topic, unique);
    topic.append(name);
    {
        const std::string id{std::string(name).append("_timestamp")};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_template].set(
            R"({"timestamp":"{{(value|as_datetime|as_local).strftime('%Y-%m-%dT%H:%M:%S')}}"})");
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::entity_category].set("config");
        component[HomeAssistantAbbreviations::icon].set("mdi:timer-sand-full");
        component[HomeAssistantAbbreviations::name].set(name);
        component[HomeAssistantAbbreviations::platform].set("datetime");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::timezone].set(TIME_ZONE);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.timestamp|as_datetime}}");
    }
}
#endif // EXTENSION_HOMEASSISTANT

#endif // MODE_COUNTDOWN
