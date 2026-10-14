#pragma once

#include <cstdint>
#include <span>
#include <string_view>

enum class OptionType : uint8_t // NOLINT(performance-enum-size)
{
    boolean,
    choice,
};

/**
 * @brief Declares one user-facing setting of a mode.
 *
 * Values are a `uint8_t`: 0 or 1 for a boolean, the index into `choices` for a choice. The payload and
 * persistence key is `key`, so changing it renames the setting for every client.
 */
struct Option
{
    std::string_view key;
    std::string_view label;
    OptionType type;
    std::string_view icon{};                     // Material Design Icons name, without the `mdi:` prefix
    std::span<const std::string_view> choices{}; // only for OptionType::choice
};
