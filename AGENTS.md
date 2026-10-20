# AGENTS.md

Notes for agents working on this fork of Frekvens. It ports modes from upstream
`ph1p/ikea-led-obegraensad` and is being revamped towards a platform-independent C++ core.
Read this before changing modes, options or the simulator.

## Direction

- Core behind a narrow platform interface (canvas, clock, random, key-value storage, state out),
  with ESP32 and host implementations. Only the simulator shims exist so far.
- Options as data: a mode declares its options once; persistence, payload handling, state
  publishing, Home Assistant entities and (eventually) the web UI derive from the declaration.
- Breaking changes are fine, including NVS layout; assume users re-flash.
- Lua, then WebAssembly, are later stretch goals. Don't start them.
- Not done: the runtime grid (see below), the platform interface headers, golden-frame tests in CI,
  a web UI driven by the `options` declaration (drop `MODE_*` flags and the `Match` registry,
  keeping Animation, Draw, Countdown, HomeThermometer).

## Options

Declared in `firmware/include/modules/Option.h`, handled by `ModeModule`
(`firmware/src/modules/ModeModule.cpp`).

- `Option{key, label, type, icon, choices, config}`; types `boolean`, `choice`, `text`.
- Boolean and choice values are `uint8_t` via `getOption(index)`/`setOption(index, value)`;
  text goes through `getText`/`setText` (return true when accepted). `choice` stores the index
  into `choices`.
- The base class loads from NVS (range-validated), publishes values at the top level of the state
  plus an `options` declaration array, handles `onReceive`, and builds the HA entity
  (`switch`, `select` or `text`, id `<Mode>_<key>`, name `<Mode> <lowercased label>`).
  `config = true` means `enabled_by_default=false` and `entity_category=config`.
- `extendState(JsonObject)` adds state that isn't an option (Weather condition, Countdown timestamp).
- Streamed-data modes (Animation, Draw, Flies, HomeThermometer) stay custom.
- `ModesService::getMode(name)` creates throwaway instances (HA discovery, boot `configure()`) and
  calls `loadOptions()`. `setOption` must not do network or display work before `begin()`; use a
  `began` flag as Snake, Ping-pong and Stream do.
- Web components (`webapp/src/modes/*.tsx`) read choice lists from the matching entry in `options`.

## Input trigger

`Trigger::active()` (`firmware/include/modules/Trigger.h`) replaces `#if EXTENSION_MICROPHONE` in
modes. Firmware returns the microphone state, or true without a microphone; the simulator returns
true. New modes use it; don't add `EXTENSION_MICROPHONE` guards.

## Grid

`GRID_COLUMNS`/`GRID_ROWS` are compile-time constants from `firmware/include/config/Ikea*.h`. Both
current devices are 16x16 and differ in pixel pitch, `LED_MAP`, model and pins. A runtime grid would
touch about 78 mode files, 450 uses, 19 array-sized headers and 83 `static_assert`s, and costs
firmware RAM and speed. Only do it for a device with another grid, or for a Lua/WASM/React renderer.

## Simulator

`tools/simulator`, host C++20 build with shims in `shims/`; the mode code is the firmware's own.

```
make -C tools/simulator
./tools/simulator/build/simulator --help
./tools/simulator/build/simulator --device frekvens --png out.png Metaballs
make -C tools/simulator check     # ASan/UBSan over every mode, 60 s each
```

- Flags go before the mode (`simulator [options] <mode>`); `--list` lists modes.
- `--device` is run-time (default `obegransad`); the table is generated from the `Ikea*.h` headers,
  so a new device header appears automatically. A device whose grid differs from the compiled one
  is refused.
- Every lamp is fitted to 512 points wide unless `--scale` is given.
- `--set KEY=VALUE`, `--state`, `--ha` exercise options, the published state and HA discovery.
- Deterministic: fixed clock, `--seed` for the RNG. That makes frame comparisons possible.
- Not simulated: Animation, Draw, Flies (need payloads), Stream, Weather, HomeThermometer
  (network). The modes list lives in `tools/simulator/Makefile` (`MODES`).
- `Ticker`'s default message is "simulator" because `NAME` is defined as that.
- A mode that reads a `DisplayService` member must have it in `shims/services/DisplayService.h`
  (for example `pixels`, which is per device).

### Checking a refactor against a previous build

Build the old commit in a separate worktree next to the repo, then compare per mode: HA JSON with
`jq -S` (`--ha`), frames with `--plain` over seeds 1 to 3, and `--state` output. Identical frames
and HA JSON mean behavior is unchanged; state payloads gain `options` by design.

## Not verified yet

- ESP32 build after the options migration (needs PlatformIO).
- TypeScript type-check of the web changes, and clang-format.
- Stream and Weather behavior, and anything on a real lamp.

Run the ESP32 build and the web type-check before relying on the migration.

## Known loose ends

- Radar draws its blips wrong in the upper half.
- No Fireflies port yet.
- `docs/wiki/Modes.md` doesn't describe the simulator.

## Repo and git

- Branch `port-obegraensad-plugins`. `origin` is upstream (`vipnytt/frekvens`); the private fork is
  remote `cgrs` (`github.com/cgrs/frekvens`, standalone since GitHub has no private forks of public repos).
- git doesn't pick up `gh`'s token here; push with
  `git -c credential.helper='!gh auth git-credential' push`. Pushing `.github/workflows/` needs the
  `workflow` scope on the token.
- Shell is fish: no bash-only constructs in ad hoc commands.

## Toolchain (mise)

`mise.toml` pins Python 3.10 and uv; PlatformIO, Node and the script dependencies come from the
`pyproject.toml` groups, which `extra_script.py` syncs itself. `mise trust && mise install` once.

```
mise run build [device] [env]      # device: obegransad (default) or frekvens; env: esp32dev by default
mise run buildfs [device] [env]    # Web app image
mise run upload [device] [env]     # USB, or Wi-Fi with upload_protocol = espota
mise run uploadfs [device] [env]
mise run flash [device] [env]      # upload + uploadfs
mise run monitor [env]
mise run lint
mise run sim                       # host simulator
mise run sim:check
```

The first build downloads the ESP32 toolchain into the gitignored `.platformio/`.

### Device, pins and Wi-Fi

The build reads only `.env` (not the process environment) and `firmware/include/config/secrets.h`, both
tracked templates. It fails with `#error "Unknown device."` without `IKEA_*` in `.env`, with `PIN_*`
errors without pins, and with `WIFI_SSID` undeclared without credentials.

`scripts/with-config.sh <device> <command>` appends the device, pins and Wi-Fi for one run and
restores both files afterwards, so the tree stays clean. The mise build tasks use it. Wi-Fi can also go in
`.env` (every key there becomes a `-D` define, and the wrapper then leaves Wi-Fi alone), but `.env` is
tracked despite the `/.env*` ignore rule, so a plain `git add -A` would commit it. Prefer
`.env.local` (really ignored, loaded by mise):

```
WIFI_SSID=name
WIFI_KEY=secret
# optional pin overrides: PIN_CS PIN_SCLK PIN_MOSI PIN_OE PIN_SW1 PIN_SW2 PIN_MIC
```

Default pins are the wiring from the `secrets.h` template: CS 1, SCLK 2, MOSI 3, OE 4, and SW2 5 for
Obegransad (SW1 5, SW2 6 for Frekvens). Builds without Wi-Fi settings use placeholders; the upload
tasks refuse to run without them.

Last check: Obegransad and Frekvens on `esp32dev` both build after the options migration (RAM 21.5%, flash 42.9%).
Other boards and real hardware were not tried.
