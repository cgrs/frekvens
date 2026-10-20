// Lamp simulator: runs a firmware mode against a virtual display and draws the frames in the terminal.
//
//   simulator [options] <mode>      run `simulator --help` for the options, `simulator --list` for the modes
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
#include <cstdlib>
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
void usage(std::FILE *out)
{
    std::fprintf(out, "usage: simulator [options] <mode>\n\n"
                      "Runs a firmware mode against a virtual lamp.\n\n"
                      "lamp:\n"
                      "  --device ID         lamp to simulate (default: obegransad), one of:");
    for (const SimDevice &device : simDevices)
    {
        std::fprintf(out, " %.*s", static_cast<int>(device.id.size()), device.id.data());
    }
    std::fprintf(out, "\n"
                      "  --scale N           window points per mm of lamp (default: fit the lamp to a 512 point width)\n\n"
                      "output (default: a window on macOS, --terminal elsewhere):\n"
                      "  --terminal          draw ANSI LEDs in the terminal in real time\n"
                      "  --plain             print the last frame as text and exit\n"
                      "  --png FILE          save the last frame as an image and exit\n"
                      "  --seconds N         length of the run (default: forever, 10 with --plain or --png)\n"
                      "  --fps N             frames drawn per second (default: 30)\n"
                      "  --fast              skip real-time pacing\n\n"
                      "mode:\n"
                      "  --set KEY=VALUE     send an option, repeatable; the keys show up in --state\n"
                      "  --state             print what the mode publishes\n"
                      "  --ha                print its Home Assistant discovery and exit\n"
                      "  --seed N            random seed (default: 1)\n"
                      "  --boot              configure every mode and publish the retained state like the device does at boot\n"
                      "  --tick MS           clock step per loop (default: 1)\n\n"
                      "  --list              list the modes\n"
                      "  -h, --help          this text\n\n"
                      "examples:\n"
                      "  simulator Snake\n"
                      "  simulator --device frekvens --terminal Metaballs\n"
                      "  simulator --set clock=0 --png snake.png Snake\n");
}

void list()
{
    for (const auto &entry : registry)
    {
        std::printf("%.*s\n", static_cast<int>(entry.name.size()), entry.name.data());
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage(stderr);
        return 1;
    }
    const char *modeName{nullptr};
    bool boot{false};
    unsigned long seconds{0UL};
    unsigned fps{30U};
    unsigned tick{1U};
    unsigned long seed{1UL};
    double scale{0.0};
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
    for (int i{1}; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        const auto value{[&]() -> const char * {
            if (i + 1 >= argc)
            {
                std::fprintf(stderr, "%s needs a value (see --help)\n", argv[i]);
                std::exit(2);
            }
            return argv[++i];
        }};
        if (arg == "-h" || arg == "--help")
        {
            usage(stdout);
            return 0;
        }
        else if (arg == "--list")
        {
            list();
            return 0;
        }
        else if (arg == "--boot")
        {
            boot = true;
        }
        else if (arg == "--fast")
        {
            fast = true;
        }
        else if (arg == "--device")
        {
            deviceId = value();
        }
        else if (arg == "--state")
        {
            state = true;
        }
        else if (arg == "--ha")
        {
            ha = true;
        }
        else if (arg == "--set")
        {
            sets.emplace_back(value());
        }
        else if (arg == "--terminal")
        {
            terminal = true;
        }
        else if (arg == "--plain")
        {
            plain = true;
        }
        else if (arg == "--png")
        {
            png = value();
        }
        else if (arg == "--seconds")
        {
            seconds = std::strtoul(value(), nullptr, 10);
        }
        else if (arg == "--fps")
        {
            fps = std::max(1U, static_cast<unsigned>(std::strtoul(value(), nullptr, 10)));
        }
        else if (arg == "--tick")
        {
            tick = std::max(1U, static_cast<unsigned>(std::strtoul(value(), nullptr, 10)));
        }
        else if (arg == "--seed")
        {
            seed = std::strtoul(value(), nullptr, 10);
        }
        else if (arg == "--scale")
        {
            scale = std::strtod(value(), nullptr);
        }
        else if (arg.starts_with("-"))
        {
            std::fprintf(stderr, "unknown option: %s (see --help)\n", argv[i]);
            return 2;
        }
        else if (modeName != nullptr)
        {
            std::fprintf(stderr, "more than one mode given: %s and %s\n", modeName, argv[i]);
            return 2;
        }
        else
        {
            modeName = argv[i];
        }
    }
    if (boot)
    {
        // What the device does at boot: every mode is created, loaded and configured (which publishes its options into
        // the retained state), then the extension list is published.
        for (const auto &candidate : registry)
        {
            std::unique_ptr<ModeModule> instance{candidate.make()};
            instance->loadOptions();
            instance->configure();
        }
        JsonDocument extensions;
        JsonArray list{extensions["list"].to<JsonArray>()};
        for (const char *extension : {"Button", "Message", "Playlist", "Screenshot", "WebSocket", "Infrared"})
        {
            list.add(extension);
        }
        Device.transmit(extensions.as<JsonObjectConst>(), "Extensions");
        std::string retained;
        serializeJson(Device.transmits, retained);
        std::printf("boot ok: retained state %zu bytes (%zu as JSON)%s\n", Device.transmits.memoryUsage(), retained.size(),
                    Device.transmits.overflowed() ? ", OVERFLOWED" : "");
        return 0;
    }
    if (modeName == nullptr)
    {
        usage(stderr);
        return 2;
    }
    if (std::strcmp(modeName, "list") == 0)
    {
        list();
        return 0;
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
        if (normalize(candidate.name) == normalize(modeName))
        {
            entry = &candidate;
        }
    }
    if (entry == nullptr)
    {
        std::fprintf(stderr, "unknown mode: %s (try --list)\n", modeName);
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
    lamp.scale = scale > 0.0 ? scale : 512.0 / lamp.widthMm; // same width for every lamp unless --scale is given
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
