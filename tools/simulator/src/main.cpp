// Lamp simulator: runs a firmware mode against a virtual display and draws the frames in the terminal.
//
//   simulator list
//   simulator <mode> [--device ID] [--seconds N] [--fps N] [--tick MS] [--seed N] [--scale PT_PER_MM]
//                    [--terminal | --plain | --png FILE] [--fast]
//                    [--set KEY=VALUE]... [--state] [--ha]
//
// --device picks the lamp from firmware/include/config/Ikea*.h (default: obegransad); it sets the window size
// and the display ratio. The grid is a compile-time constant of the modes, so a device with another grid is refused.
//
// --set sends an option payload to the mode, --state prints what the mode publishes and --ha prints its Home
// Assistant discovery components.
//
// On macOS the default is a window sized from the lamp's pixel pitch; --terminal draws ANSI LEDs instead,
// --plain prints the last frame as text and --png FILE saves it as an image (both run without real-time pacing).
//
// The mode code is the firmware's own; only the display, clock and random source are replaced.

#include "SimDevice.h"
#include "modules/ModeModule.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "Window.h"

#include "registry.inc"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <thread>

namespace
{
volatile std::sig_atomic_t stop{0};

std::string normalize(std::string_view text)
{
    std::string out;
    for (const char c : text)
    {
        if (c != ' ' && c != '-')
        {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    return out;
}

void draw(std::string_view name, unsigned long ms, bool plain)
{
    if (plain)
    {
        constexpr std::string_view ramp{" .:-=+*#%@"};
        for (size_t y{0U}; y < GRID_ROWS; ++y)
        {
            for (size_t x{0U}; x < GRID_COLUMNS; ++x)
            {
                const uint8_t b{Display.getPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y))};
                std::putchar(ramp[b * (ramp.size() - 1U) / UINT8_MAX]);
                std::putchar(' ');
            }
            std::putchar('\n');
        }
        return;
    }
    std::string out{"\x1b[H"};
    for (size_t y{0U}; y < GRID_ROWS; ++y)
    {
        for (size_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            const unsigned b{Display.getPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y))};
            // The firmware applies gamma to the PWM, so brightness already reads as linear light.
            const unsigned r{b == 0U ? 34U : 34U + ((255U - 34U) * b / UINT8_MAX)};
            const unsigned g{b == 0U ? 34U : 34U + ((244U - 34U) * b / UINT8_MAX)};
            const unsigned bl{b == 0U ? 34U : 34U + ((224U - 34U) * b / UINT8_MAX)};
            char cell[48];
            std::snprintf(cell, sizeof(cell), "\x1b[38;2;%u;%u;%um● ", r, g, bl);
            out += cell;
        }
        out += "\x1b[0m\n";
    }
    char footer[96];
    std::snprintf(footer, sizeof(footer), "\x1b[0m%.*s  %lu.%01lus  (ctrl-c to quit)\x1b[K\n", static_cast<int>(name.size()),
                  name.data(), ms / 1000UL, (ms % 1000UL) / 100UL);
    out += footer;
    std::fwrite(out.data(), 1U, out.size(), stdout);
    std::fflush(stdout);
}
} // namespace

int main(int argc, char **argv)
{
    if (argc < 2 || std::strcmp(argv[1], "list") == 0 || std::strcmp(argv[1], "--help") == 0)
    {
        for (const auto &entry : registry)
        {
            std::printf("%.*s\n", static_cast<int>(entry.name.size()), entry.name.data());
        }
        return argc < 2 ? 1 : 0;
    }
    unsigned long seconds{0UL};
    unsigned fps{30U};
    unsigned tick{1U};
    unsigned long seed{1UL};
    double scale{1.6};
    bool fast{false};
    bool plain{false};
#ifdef __APPLE__
    bool terminal{false};
#else
    bool terminal{true};
#endif
    const char *png{nullptr};
    std::vector<std::string> sets;
    bool state{false};
    bool ha{false};
    std::string_view deviceId{};
    for (int i{2}; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        if (arg == "--fast")
        {
            fast = true;
        }
        else if (i + 1 < argc && arg == "--device")
        {
            deviceId = argv[++i];
        }
        else if (arg == "--state")
        {
            state = true;
        }
        else if (arg == "--ha")
        {
            ha = true;
        }
        else if (i + 1 < argc && arg == "--set")
        {
            sets.emplace_back(argv[++i]);
        }
        else if (arg == "--terminal")
        {
            terminal = true;
        }
        else if (arg == "--plain")
        {
            plain = true;
        }
        else if (i + 1 < argc && arg == "--png")
        {
            png = argv[++i];
        }
        else if (i + 1 < argc && arg == "--seconds")
        {
            seconds = std::strtoul(argv[++i], nullptr, 10);
        }
        else if (i + 1 < argc && arg == "--fps")
        {
            fps = std::max(1U, static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10)));
        }
        else if (i + 1 < argc && arg == "--tick")
        {
            tick = std::max(1U, static_cast<unsigned>(std::strtoul(argv[++i], nullptr, 10)));
        }
        else if (i + 1 < argc && arg == "--seed")
        {
            seed = std::strtoul(argv[++i], nullptr, 10);
        }
        else if (i + 1 < argc && arg == "--scale")
        {
            scale = std::strtod(argv[++i], nullptr);
        }
        else
        {
            std::fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }
    if (!deviceId.empty())
    {
        const auto *found{std::ranges::find_if(simDevices, [deviceId](const SimDevice &device) { return device.id == deviceId; })};
        if (found == std::end(simDevices))
        {
            std::fprintf(stderr, "unknown device: %.*s (known:", static_cast<int>(deviceId.size()), deviceId.data());
            for (const SimDevice &device : simDevices)
            {
                std::fprintf(stderr, " %.*s", static_cast<int>(device.id.size()), device.id.data());
            }
            std::fprintf(stderr, ")\n");
            return 2;
        }
        simDevice = found;
    }
    if (simDevice->columns != GRID_COLUMNS || simDevice->rows != GRID_ROWS)
    {
        std::fprintf(stderr, "%.*s has a %ux%u grid; this build of the modes is %ux%u\n", static_cast<int>(simDevice->id.size()),
                     simDevice->id.data(), simDevice->columns, simDevice->rows, static_cast<unsigned>(GRID_COLUMNS),
                     static_cast<unsigned>(GRID_ROWS));
        return 2;
    }
    DisplayService::pixels = simDevice->ledMap;
    const bool offscreen{plain || png != nullptr};
    if (offscreen)
    {
        fast = true;
        if (seconds == 0UL)
        {
            seconds = 10UL;
        }
    }
    const Entry *entry{nullptr};
    for (const auto &candidate : registry)
    {
        if (normalize(candidate.name) == normalize(argv[1]))
        {
            entry = &candidate;
        }
    }
    if (entry == nullptr)
    {
        std::fprintf(stderr, "unknown mode: %s (try `list`)\n", argv[1]);
        return 2;
    }
    randomSeed(seed);
    std::unique_ptr<ModeModule> mode{entry->make()};
    mode->loadOptions(); // ModesService::getMode() does this for every mode it creates
    Device.verbose = state;
    if (ha)
    {
        JsonDocument discovery;
        mode->onHomeAssistant(discovery, "frekvens/", "unique_");
        std::string json;
        serializeJsonPretty(discovery, json);
        std::printf("%s\n", json.c_str());
        return 0;
    }
    // Same order as the firmware: configure() at boot, option payloads as they arrive, then begin().
    Display.fillFrame(0U);
    mode->configure();
    for (const std::string &set : sets)
    {
        const size_t eq{set.find('=')};
        if (eq == std::string::npos)
        {
            std::fprintf(stderr, "--set needs KEY=VALUE: %s\n", set.c_str());
            return 2;
        }
        const std::string key{set.substr(0U, eq)};
        const std::string value{set.substr(eq + 1U)};
        JsonDocument payload;
        if (value == "true" || value == "false")
        {
            payload[key].set(value == "true");
        }
        else if (!value.empty() && value.find_first_not_of("0123456789") == std::string::npos)
        {
            payload[key].set(std::strtol(value.c_str(), nullptr, 10));
        }
        else
        {
            payload[key].set(value);
        }
        mode->onReceive(payload.as<JsonObjectConst>(), "simulator");
    }
    mode->begin();

    const unsigned frameMs{1000U / fps};
    unsigned long elapsed{0UL};
    // The firmware task calls handle() then yields for one tick, so step the clock the same way.
    const auto step{[&]() -> bool {
        if (stop != 0 || (seconds != 0UL && elapsed >= seconds * 1000UL))
        {
            return false;
        }
        for (unsigned t{0U}; t < frameMs; t += tick)
        {
            simAdvance(tick);
            mode->handle();
            Display.flush();
        }
        elapsed += frameMs;
        return true;
    }};

    LampWindow lamp{};
    lamp.title = std::string{simDevice->title} + " - " + std::string{entry->name};
    lamp.columns = GRID_COLUMNS;
    lamp.rows = GRID_ROWS;
    lamp.widthMm = static_cast<double>(GRID_COLUMNS * simDevice->pitchHorizontal);
    lamp.heightMm = static_cast<double>(GRID_ROWS * simDevice->pitchVertical);
    lamp.scale = scale;
    lamp.fps = fps;
    lamp.step = step;
    lamp.pixel = [](unsigned x, unsigned y) { return Display.getPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y)); };

    if (png != nullptr || offscreen)
    {
        while (step())
        {
        }
        if (plain)
        {
            draw(entry->name, elapsed, true);
        }
#ifdef __APPLE__
        if (png != nullptr && !savePng(lamp, png))
        {
            std::fprintf(stderr, "could not write %s\n", png);
            return 1;
        }
#else
        if (png != nullptr)
        {
            std::fprintf(stderr, "--png needs macOS\n");
            return 1;
        }
#endif
        return 0;
    }
#ifdef __APPLE__
    if (!terminal)
    {
        return runWindow(lamp);
    }
#endif
    std::signal(SIGINT, [](int) { stop = 1; });
    std::fputs("\x1b[2J\x1b[?25l", stdout);
    while (true)
    {
        const auto started{std::chrono::steady_clock::now()};
        if (!step())
        {
            break;
        }
        draw(entry->name, elapsed, false);
        if (!fast)
        {
            std::this_thread::sleep_until(started + std::chrono::milliseconds(frameMs));
        }
    }
    std::fputs("\x1b[?25h\x1b[0m", stdout);
    return 0;
}
