# Host build of the firmware modes. Needs only a C++20 compiler.
#   make            build ./build/simulator
#   ./build/simulator --device frekvens <mode>   picks the lamp at run time (--help for the rest)
# Not simulated: Animation, Countdown, Draw, Flies (need payloads to show anything), Stream, Weather and
# HomeThermometer (network). The rest of the firmware modes run on the shims in shims/.

CXX ?= clang++
ROOT := ../..
FW := $(ROOT)/firmware
BUILD := build

MODES := Countdown Arrow AutoWalker BinaryEpoch Blinds Blink BreakoutClock Bright Circle Clock Equalizer Firework GameOfLife Glitter LeafFall Lines Noise PingPong PixelSequence Ring Scan Snake Ticker Waveform BigPong BouncingBall Bubbles Checkerboard Comet Face FallingSand FlappyBird MatrixRain \
	MeteorShower Metaballs PongClock Radar Rain Scanlines SpaceInvaders SparkleField Spiral Spotlight Stars \
	Tetris Tron Wave WaveBars

DEFS := -DTIME_ZONE='"UTC"' -DEXTENSION_HOMEASSISTANT=1 -DNAME='"simulator"' -DPIN_CS=0 -DPIN_SCLK=1 -DPIN_MOSI=2 -DPIN_OE=3 $(foreach m,$(MODES),-DMODE_$(shell echo $(m) | tr a-z A-Z)=1)
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-reorder-ctor -Wno-sign-compare -Wno-unused-private-field -include Arduino.h -Ishims -Ibuild -I$(BUILD) -I$(FW)/include $(DEFS)
SRCS := shims/Arduino.cpp shims/Trigger.cpp shims/DisplayService.cpp shims/nvs.cpp \
	$(FW)/src/modules/ModeModule.cpp $(FW)/src/modules/FontModule.cpp $(FW)/src/modules/HandlerModule.cpp $(FW)/src/modules/ServiceModule.cpp \
	$(FW)/src/handlers/ClockHandler.cpp $(FW)/src/handlers/TextHandler.cpp $(FW)/src/handlers/BitmapHandler.cpp \
	$(wildcard $(FW)/src/fonts/*.cpp) $(FW)/src/services/FontsService.cpp \
	$(foreach m,$(MODES),$(FW)/src/modes/$(m)Mode.cpp) src/main.cpp
ifeq ($(shell uname -s),Darwin)
SRCS += src/Window.mm
LDFLAGS := -framework Cocoa
endif

$(BUILD)/simulator: $(SRCS) src/Window.h build/ArduinoJson.h $(BUILD)/devices.inc $(BUILD)/registry.inc $(BUILD)/DisplayDraw.inc $(BUILD)/HomeAssistantAbbreviations.inc
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $@

$(BUILD)/DisplayDraw.inc: $(FW)/src/services/DisplayService.cpp | $(BUILD)
	awk '/^uint8_t DisplayService::getPixel\(size_t idx\)/ {on=1} on {print} /^void DisplayService::invertFrame/ {inv=1} inv && /^}/ {exit}' $< > $@

# One row per device header; the pitch is what tells the lamps apart, so the grid must agree with the build.
$(BUILD)/devices.inc: Makefile $(wildcard $(FW)/include/config/Ikea*.h) | $(BUILD)
	for f in $(wildcard $(FW)/include/config/Ikea*.h); do id=$$(basename $$f .h | sed 's/^Ikea//' | tr A-Z a-z); \
	  awk -v id=$$id '/^#define MANUFACTURER/ {ma=$$3} /^#define MODEL/ {mo=$$3} /^#define GRID_COLUMNS/ {c=$$3} /^#define GRID_ROWS/ {r=$$3} /^#define PITCH_HORIZONTAL/ {ph=$$3} /^#define LED_MAP/ {led=1; next} led {t=$$0; gsub(/[ \\\\\t]/, "", t); if (t == "}") {led=0} else if (t != "{") {map=map t}} /^#define PITCH_VERTICAL/ {pv=$$3} END {printf "{\"%s\", %s \" \" %s, %s, %s, %s, %s, {%s}},\n", id, ma, mo, c, r, ph, pv, map}' $$f; done > $@

$(BUILD)/registry.inc: Makefile | $(BUILD)
	{ for m in $(MODES); do echo "#include \"modes/$${m}Mode.h\""; done; \
	  echo '#include <functional>'; echo 'struct Entry { std::string_view name; std::unique_ptr<ModeModule> (*make)(); };'; \
	  echo 'static const Entry registry[] = {'; \
	  for m in $(MODES); do u=$$(echo $$m | tr a-z A-Z); \
	    echo "#if MODE_$$u"; echo "  {$${m}Mode::name, []() -> std::unique_ptr<ModeModule> { return std::make_unique<$${m}Mode>(); }},"; echo "#endif"; done; \
	  echo '};'; } > $@

$(BUILD):
	mkdir -p $@

clean:
	find build -mindepth 1 -maxdepth 1 ! -name ArduinoJson.h -exec rm -rf {} +

.PHONY: clean

# Run every mode headless for 60 virtual seconds under ASan/UBSan; fails on out-of-range writes and the like.
check: $(SRCS) build/ArduinoJson.h $(BUILD)/devices.inc $(BUILD)/registry.inc $(BUILD)/DisplayDraw.inc $(BUILD)/HomeAssistantAbbreviations.inc
	$(CXX) $(CXXFLAGS) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all $(SRCS) $(LDFLAGS) -o $(BUILD)/simulator-check
	@$(BUILD)/simulator-check --list | while read -r mode; do \
	  $(BUILD)/simulator-check --plain --seconds 60 "$$mode" >/dev/null || { echo "FAIL: $$mode"; exit 1; }; \
	  echo "ok: $$mode"; done

.PHONY: check

# The modes parse payloads with ArduinoJson; fetch the same version the firmware pins (single header, cached).
ARDUINOJSON := 7.4.3
build/ArduinoJson.h:
	mkdir -p build
	curl -fsSL -o $@ https://github.com/bblanchon/ArduinoJson/releases/download/v$(ARDUINOJSON)/ArduinoJson-v$(ARDUINOJSON).h

$(BUILD)/HomeAssistantAbbreviations.inc: $(FW)/include/extensions/HomeAssistantExtension.h | $(BUILD)
	awk '/^namespace HomeAssistantAbbreviations/ {on=1} on {print} on && /^}/ {exit}' $< > $@
