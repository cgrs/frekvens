#pragma once

#include <ArduinoJson.h>
#include <string_view>

// Modes publish their state through Device.transmit(); the simulator has nowhere to send it.
class DeviceService
{
public:
    void transmit(JsonObjectConst payload, std::string_view source, bool retain = true) {}
};

extern DeviceService &Device;
