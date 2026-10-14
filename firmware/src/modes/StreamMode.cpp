#if MODE_STREAM

#include "modes/StreamMode.h"

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "services/DisplayService.h"

/**
 * @brief Starts UDP listening on the configured port and registers its protocol handler.
 */
void StreamMode::begin()
{
    began = true;
    const uint16_t port{ports[protocol]};
    if (udp.listen(port))
    {
        switch (port)
        {
        case 4048U:
            udp.onPacket(&onDistributedDisplayProtocol);
            break;
        case 5568U:
            udp.onPacket(&onE131);
            break;
        case 6454U:
            udp.onPacket(&onArtNet);
            break;
        default:
            return;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        ESP_LOGD(name.data(), "listening at " HOSTNAME ".local:%u", port);
    }
}

/**
 * @brief Gets the selected protocol.
 *
 * @param index Index into the declared options.
 */
uint8_t StreamMode::getOption(size_t index) const { return protocol; }

/**
 * @brief Selects the streaming protocol and restarts UDP listening when the mode is running.
 *
 * @param index Index into the declared options.
 * @param value Index into the protocols.
 */
void StreamMode::setOption(size_t index, uint8_t value)
{
    protocol = value;
    if (began)
    {
        begin();
    }
}

/**
 * @brief Processes an Art-Net packet containing one complete display frame.
 *
 * @param packet UDP packet with an 18-byte Art-Net header followed by the frame data.
 */
void StreamMode::onArtNet(AsyncUDPPacket packet)
{
    if (packet.length() == 18U + (GRID_COLUMNS * GRID_ROWS))
    {
        Display.setFrame(static_cast<std::span<const uint8_t, GRID_COLUMNS * GRID_ROWS>>(
            std::span(packet.data(), packet.length()).subspan(18U)));
    }
}

/**
 * @brief Processes a Distributed Display Protocol packet and updates the display frame.
 *
 * @param packet UDP packet containing a supported protocol header followed by a complete display frame.
 */
void StreamMode::onDistributedDisplayProtocol(AsyncUDPPacket packet)
{
    const std::span<const uint8_t> data{std::span(packet.data(), packet.length())};
    if (packet.length() == 10U + (GRID_COLUMNS * GRID_ROWS) && (data.front() & (0b1U << 4U)) == 0U)
    {
        Display.setFrame(static_cast<std::span<const uint8_t, GRID_COLUMNS * GRID_ROWS>>(
            std::span(packet.data(), packet.length()).subspan(10U)));
    }
    else if (packet.length() == 14U + (GRID_COLUMNS * GRID_ROWS) && (data.front() & (0b1U << 4U)) != 0U)
    {
        Display.setFrame(static_cast<std::span<const uint8_t, GRID_COLUMNS * GRID_ROWS>>(
            std::span(packet.data(), packet.length()).subspan(14U)));
    }
}

/**
 * @brief Processes an E1.31 packet and updates the display frame.
 *
 * @param packet UDP packet containing a 126-byte E1.31 header followed by a complete display frame.
 */
void StreamMode::onE131(AsyncUDPPacket packet)
{
    if (packet.length() == 126U + (GRID_COLUMNS * GRID_ROWS))
    {
        Display.setFrame(static_cast<std::span<const uint8_t, GRID_COLUMNS * GRID_ROWS>>(
            std::span(packet.data(), packet.length()).subspan(126U)));
    }
}

#endif // MODE_STREAM
