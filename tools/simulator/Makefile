# Host build of the firmware modes. Needs only a C++20 compiler.
#   make            build ./build/obegransad/simulator
#   make DEVICE=frekvens   (build/frekvens/simulator)
# Not simulated: Animation, Countdown, Draw, Flies (need payloads to show anything), Stream, Weather and
# HomeThermometer (network). The rest of the firmware modes run on the shims in shims/.

DEVICE ?= obegransad
CXX ?= clang++
ROOT := ../..
FW := $(ROOT)/firmware
BUILD := build/$(DEVICE)

MODES := Arrow AutoWalker BinaryEpoch Blinds Blink BreakoutClock Bright Circle Clock Equalizer Firework GameOfLife Glitter LeafFall Lines Noise PingPong PixelSequence Ring Scan Snake Ticker Waveform BigPong BouncingBall Bubbles Checkerboard Comet Face FallingSand FlappyBird MatrixRain \
	MeteorShower Metaballs PongClock Radar Rain Scanlines SpaceInvaders SparkleField Spiral Spotlight Stars \
	Tetris Tron Wave WaveBars

DEFS := -DEXTENSION_HOMEASSISTANT=1 -DNAME='"$(DEVICE)"' -DPIN_CS=0 -DPIN_SCLK=1 -DPIN_MOSI=2 -DPIN_OE=3 $(foreach m,$(MODES),-DMODE_$(shell echo $(m) | tr a-z A-Z)=1) -DSIM_FREKVENS=$(if $(filter frekvens,$(DEVICE)),1,0) -DSIM_DEVICE_NAME='"$(if $(filter frekvens,$(DEVICE)),IKEA Frekvens,IKEA Obegränsad)"'
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-reorder-ctor -Wno-sign-compare -Wno-unused-private-field -include Arduino.h -Ishims -Ibuild -I$(BUILD) -I$(FW)/include $(DEFS)
SRCS := shims/Arduino.cpp shims/DisplayService.cpp shims/nvs.cpp \
	$(FW)/src/modules/ModeModule.cpp $(FW)/src/modules/FontModule.cpp $(FW)/src/modules/HandlerModule.cpp $(FW)/src/modules/ServiceModule.cpp \
	$(FW)/src/handlers/ClockHandler.cpp $(FW)/src/handlers/TextHandler.cpp $(FW)/src/handlers/BitmapHandler.cpp \
	$(wildcard $(FW)/src/fonts/*.cpp) $(FW)/src/services/FontsService.cpp \
	$(foreach m,$(MODES),$(FW)/src/modes/$(m)Mode.cpp) src/main.cpp
ifeq ($(shell uname -s),Darwin)
SRCS += src/Window.mm
LDFLAGS := -framework Cocoa
endif

$(BUILD)/simulator: $(SRCS) src/Window.h build/ArduinoJson.h $(BUILD)/registry.inc $(BUILD)/DisplayDraw.inc $(BUILD)/HomeAssistantAbbreviations.inc
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $@

$(BUILD)/DisplayDraw.inc: $(FW)/src/services/DisplayService.cpp | $(BUILD)
	awk '/^uint8_t DisplayService::getPixel\(size_t idx\)/ {on=1} on {print} /^void DisplayService::invertFrame/ {inv=1} inv && /^}/ {exit}' $< > $@

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
	rm -rf $(BUILD)

.PHONY: clean

# Run every mode headless for 60 virtual seconds under ASan/UBSan; fails on out-of-range writes and the like.
check: $(SRCS) build/ArduinoJson.h $(BUILD)/registry.inc $(BUILD)/DisplayDraw.inc $(BUILD)/HomeAssistantAbbreviations.inc
	$(CXX) $(CXXFLAGS) -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all $(SRCS) $(LDFLAGS) -o $(BUILD)/simulator-check
	@$(BUILD)/simulator-check list | while read -r mode; do \
	  $(BUILD)/simulator-check "$$mode" --plain --seconds 60 >/dev/null || { echo "FAIL: $$mode"; exit 1; }; \
	  echo "ok: $$mode"; done

.PHONY: check

# The modes parse payloads with ArduinoJson; fetch the same version the firmware pins (single header, cached).
ARDUINOJSON := 7.4.3
build/ArduinoJson.h:
	mkdir -p build
	curl -fsSL -o $@ https://github.com/bblanchon/ArduinoJson/releases/download/v$(ARDUINOJSON)/ArduinoJson-v$(ARDUINOJSON).h

$(BUILD)/HomeAssistantAbbreviations.inc: $(FW)/include/extensions/HomeAssistantExtension.h | $(BUILD)
	awk '/^namespace HomeAssistantAbbreviations/ {on=1} on {print} on && /^}/ {exit}' $< > $@
