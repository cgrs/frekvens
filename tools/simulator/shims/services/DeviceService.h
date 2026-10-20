#pragma once

#include <ArduinoJson.h>
#include <string_view>

// Modes publish their state through Device.transmit(); the simulator prints it when asked to.
class DeviceService
{
public:
    bool verbose{false};
    JsonDocument transmits; // retained payloads, as on the device

    void transmit(JsonObjectConst payload, std::string_view source, bool retain = true);
};

extern DeviceService &Device;
