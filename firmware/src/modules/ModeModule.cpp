#include "modules/ModeModule.h"

#include "extensions/HomeAssistantExtension.h" // NOLINT(misc-include-cleaner)
#include "services/DeviceService.h"

#include <algorithm>
#include <cctype>
#include <nvs.h>
#include <string>

/**
 * @brief Publishes the option values and their declaration.
 */
void ModeModule::configure() { transmitOptions(); }
void ModeModule::begin() {}
void ModeModule::handle() {}
void ModeModule::end() {}

std::span<const Option> ModeModule::options() const { return {}; }
uint8_t ModeModule::getOption(size_t index) const { return 0U; }
void ModeModule::setOption(size_t index, uint8_t value) {}

/**
 * @brief Restores the persisted option values.
 *
 * Stored as `uint8_t` under the option's key in the mode's namespace. Values outside a choice's range are ignored.
 */
void ModeModule::loadOptions()
{
    const std::span<const Option> _options{options()};
    nvs_handle_t handle{};
    if (_options.empty() || nvs_open(name.data(), nvs_open_mode_t::NVS_READONLY, &handle) != ESP_OK)
    {
        return;
    }
    for (size_t idx{0U}; idx < _options.size(); ++idx)
    {
        uint8_t value{0U};
        if (nvs_get_u8(handle, _options[idx].key.data(), &value) == ESP_OK &&
            (_options[idx].type == OptionType::boolean ? value <= 1U : value < _options[idx].choices.size()))
        {
            setOption(idx, value);
        }
    }
    nvs_close(handle);
}

/**
 * @brief Publishes the current option values together with the option declarations.
 *
 * Values keep their key at the top level; the declarations go in `options`. They are sent together because
 * the device retains only the latest payload per source.
 */
void ModeModule::transmitOptions() const
{
    const std::span<const Option> _options{options()};
    if (_options.empty())
    {
        return;
    }
    JsonDocument doc{};
    JsonArray declarations{doc["options"].to<JsonArray>()};
    for (size_t idx{0U}; idx < _options.size(); ++idx)
    {
        const Option &option{_options[idx]};
        const uint8_t value{getOption(idx)};
        JsonObject declaration{declarations.add<JsonObject>()};
        declaration["key"].set(option.key);
        declaration["label"].set(option.label);
        if (!option.icon.empty())
        {
            declaration["icon"].set(option.icon);
        }
        if (option.type == OptionType::boolean)
        {
            doc[option.key.data()].set(value != 0U);
            declaration["type"].set("boolean");
        }
        else
        {
            doc[option.key.data()].set(option.choices[std::min<size_t>(value, option.choices.size() - 1U)]);
            declaration["type"].set("choice");
            JsonArray choices{declaration["choices"].to<JsonArray>()};
            for (const std::string_view choice : option.choices)
            {
                choices.add(choice);
            }
        }
    }
    Device.transmit(doc.as<JsonObjectConst>(), name);
}

/**
 * @brief Applies, persists and publishes the options present in a payload.
 *
 * @param payload Received JSON payload.
 * @param source Identifier of the payload source.
 */
void ModeModule::onReceive(JsonObjectConst payload, std::string_view source)
{
    const std::span<const Option> _options{options()};
    bool changed{false};
    for (size_t idx{0U}; idx < _options.size(); ++idx)
    {
        const Option &option{_options[idx]};
        const JsonVariantConst field{payload[option.key.data()]};
        uint8_t value{0U};
        if (option.type == OptionType::boolean)
        {
            if (!field.is<bool>())
            {
                continue;
            }
            value = field.as<bool>() ? 1U : 0U;
        }
        else
        {
            if (!field.is<std::string_view>())
            {
                continue;
            }
            const auto found{std::ranges::find(option.choices, field.as<std::string_view>())};
            if (found == option.choices.end())
            {
                continue;
            }
            value = static_cast<uint8_t>(found - option.choices.begin());
        }
        setOption(idx, value);
        nvs_handle_t handle{};
        if (nvs_open(name.data(), nvs_open_mode_t::NVS_READWRITE, &handle) == ESP_OK)
        {
            nvs_set_u8(handle, option.key.data(), value);
            nvs_commit(handle);
            nvs_close(handle);
        }
        changed = true;
    }
    if (changed)
    {
        transmitOptions();
    }
}

#if EXTENSION_HOMEASSISTANT
/**
 * @brief Adds a switch or select entity per option to the Home Assistant discovery data.
 *
 * @param discovery Home Assistant discovery document to update.
 * @param topic Base topic used for the entities' command and state topics.
 * @param unique Prefix used to construct the entities' unique identifiers.
 */
void ModeModule::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    for (const Option &option : options())
    {
        const std::string key{option.key};
        const std::string id{std::string(name).append("_").append(key)};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::enabled_by_default].set(false);
        component[HomeAssistantAbbreviations::entity_category].set("config");
        if (!option.icon.empty())
        {
            component[HomeAssistantAbbreviations::icon].set(std::string("mdi:").append(option.icon));
        }
        std::string label{option.label};
        std::ranges::transform(label, label.begin(), [](unsigned char chr) { return std::tolower(chr); });
        component[HomeAssistantAbbreviations::name].set(std::string(name).append(" ").append(label));
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json." + key + "}}");
        if (option.type == OptionType::boolean)
        {
            component[HomeAssistantAbbreviations::command_template].set("{\"" + key + "\":{{value}}}");
            component[HomeAssistantAbbreviations::payload_off].set("false");
            component[HomeAssistantAbbreviations::payload_on].set("true");
            component[HomeAssistantAbbreviations::platform].set("switch");
            component[HomeAssistantAbbreviations::state_off].set("False");
            component[HomeAssistantAbbreviations::state_on].set("True");
        }
        else
        {
            component[HomeAssistantAbbreviations::command_template].set("{\"" + key + "\":\"{{value}}\"}");
            JsonArray choices{component[HomeAssistantAbbreviations::options].to<JsonArray>()};
            for (const std::string_view choice : option.choices)
            {
                choices.add(choice);
            }
            component[HomeAssistantAbbreviations::platform].set("select");
        }
    }
}
#endif // EXTENSION_HOMEASSISTANT
