#pragma once

#include "modules/Option.h"

#include <ArduinoJson.h> // NOLINT(misc-include-cleaner)
#include <span>
#include <string_view>

class ModeModule
{
protected:
    explicit ModeModule(std::string_view name) : name(name) {};

public:
    virtual ~ModeModule() = default;

    ModeModule(const ModeModule &) = delete;
    ModeModule &operator=(const ModeModule &) = delete;
    ModeModule(ModeModule &&) = delete;
    ModeModule &operator=(ModeModule &&) = delete;

    const std::string_view name{};

    virtual void configure();
    virtual void begin();
    virtual void handle();
    virtual void end();

    /**
     * Settings the mode exposes. Persistence, payload handling, state publishing and the Home Assistant
     * entities are derived from this list; a mode only stores and applies the values.
     */
    [[nodiscard]] virtual std::span<const Option> options() const;
    [[nodiscard]] virtual uint8_t getOption(size_t index) const;
    virtual void setOption(size_t index, uint8_t value);

    void loadOptions();
    void transmitOptions() const;

    virtual void onReceive(JsonObjectConst payload, std::string_view source);

#if EXTENSION_HOMEASSISTANT
    virtual void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique);
#endif
};
