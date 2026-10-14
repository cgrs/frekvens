#pragma once

#if MODE_STREAM

#include "modules/ModeModule.h"

#include <AsyncUDP.h>
#include <array>
#include <bits/unique_ptr.h>
#include <span>

class StreamMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 3U> protocolNames{
        "Distributed Display Protocol",
        "E1.31",
        "Art-Net",
    };

    static constexpr std::array<uint16_t, 3U> ports{4048U, 5568U, 6454U};

    static constexpr std::array<Option, 1U> declared{
        Option{"protocol", "Protocol", OptionType::choice, "protocol", protocolNames},
    };

    bool began{false};

    uint8_t protocol{0U};

    AsyncUDP udp{};

    static void onArtNet(AsyncUDPPacket packet);
    static void onDistributedDisplayProtocol(AsyncUDPPacket packet);
    static void onE131(AsyncUDPPacket packet);

public:
    static constexpr std::string_view name{"Stream"};

    explicit StreamMode() : ModeModule(name) {};

    void begin() override;

    [[nodiscard]] std::span<const Option> options() const override { return declared; }
    [[nodiscard]] uint8_t getOption(size_t index) const override;
    void setOption(size_t index, uint8_t value) override;
};

#endif // MODE_STREAM
