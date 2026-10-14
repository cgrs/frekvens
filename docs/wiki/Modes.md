# 🎛️ Modes

**Clocks:** [Binary epoch](#1%EF%B8%8F⃣-binary-epoch) | [Breakout clock](#-breakout-clock) | [Clock](#️-clock) | [Game of Life](#-game-of-life) | [Ping-pong](#-ping-pong) | [Snake](#️-snake) | [Pong clock](#-pong-clock)

**Games:** [Breakout clock](#-breakout-clock) | [Game of life](#-game-of-life) | [Ping-pong](#-ping-pong)| [Snake](#-snake) | [Big pong](#-big-pong) | [Flappy bird](#-flappy-bird) | [Pong clock](#-pong-clock) | [Space invaders](#-space-invaders) | [Tetris](#-tetris) | [Tron](#️-tron)

**IKEA Frekvens:** [Arrow](#️-arrow) | [Blinds](#-blinds) | [Equalizer](#-equalizer) | [Ring](#-ring) | [Waveform](#-waveform)

**IKEA Obegränsad:** [Circle](#-circle) | [Lines](#-lines) | [Noise](#-noise) | [Rain](#-rain) | [Scan](#-scan) | [Stars](#-stars)

**Interactive:** [Animation](#-animation) | [Countdown](#-countdown) | [Draw](#-draw) | [Flies](#-flies) | [Ticker](#🅰%EF%B8%8F-ticker)

**Smart-home:** [Home Thermometer](#️-home-thermometer) | [Stream](#-stream) | [Weather](#-weather)

**Animations:** [Auto walker](#-auto-walker) | [Bouncing ball](#-bouncing-ball) | [Bubbles](#-bubbles) | [Checkerboard](#-checkerboard) | [Comet](#️-comet) | [Face](#-face) | [Falling sand](#-falling-sand) | [Matrix rain](#-matrix-rain) | [Meteor shower](#-meteor-shower) | [Radar](#-radar) | [Scanlines](#-scanlines) | [Sparkle field](#-sparkle-field) | [Spiral](#-spiral) | [Spotlight](#-spotlight) | [Wave](#-wave) | [Wave bars](#-wave-bars)

**Miscellaneous:** [Blink](#️-blink) | [Bright](#-bright) | [Firework](#-firework) | [Glitter](#-glitter) | [Leaf fall](#-leaf-fall) | [Metaballs](#-metaballs) | [Pixel sequence](#-pixel-sequence) | [Waveform](#-waveform)

## 🎬 Animation

Create simple animations and show it off on the display.

Use the [Web app](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-web-app) to get started.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the animations.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_ANIMATION='true'
```

See also [Draw](#-draw).

## ⬇️ Arrow

Inspired by the original *IKEA Frekvens* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the arrow, just like on a stock *IKEA Frekvens* device.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_ARROW='true'
```

## 🚶 Auto walker

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a walker prefers the least visited neighbouring pixel and paints a trail that brightens with each visit, until every pixel has been visited.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_AUTOWALKER='true'
```

## 🎮 Big pong

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: self-playing pong with wide paddles at the top and bottom, where the ball speeds up with each hit.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BIGPONG='true'
```

## 1️⃣ Binary epoch

Binary clock, counts seconds since 1970.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BINARYEPOCH='true'
```

## 🪟 Blinds

Inspired by the original *IKEA Frekvens* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the blinds.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BLINDS='true'
```

## 👁️ Blink

Use it as an alarm.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BLINK='true'
```

See also [Bright](#-bright).

## 🏀 Bouncing ball

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a ball bounces down through randomly generated obstacles, then a new level starts.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BOUNCINGBALL='true'
```

## 💥 Breakout clock

Inspired by the classic breakout game.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the game.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BREAKOUT='true'
```

## 🌟 Bright

Use it as a lamp, if desired.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BRIGHT='true'
```

See also [Blink](#%EF%B8%8F-blink).

## 🫧 Bubbles

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: ring-shaped bubbles that grow and pop at random positions.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_BUBBLES='true'
```

## 🏁 Checkerboard

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a checkerboard that cross-fades and inverts, occasionally shifting by one pixel.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_CHECKERBOARD='true'
```

## ⚪ Circle

Inspired by the original *IKEA Obegränsad* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the circle.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_CIRCLE='true'
```

See also [Ring](#-ring).

## ⏱️ Clock

Clock with font selection and optional seconds indicator.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_CLOCK='true'
```

## ☄️ Comet

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a comet bouncing diagonally around the display, leaving a fading trail.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_COMET='true'
```

## ⏳ Countdown

Start a countdown timer.

The display will blink when the timer reaches zero.

Use the [Web app](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-web-app) to start a new countdown, it can also be automated via [Home Assistant](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-home-assistant).

API payload examples:

```json
{
    "time": 600
}
```

```json
{
    "timestamp": "2025-12-02T18:39:00"
}
```

> [!NOTE]
> The expected timestamp format is an ISO 8601 string without timezone suffix.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_COUNTDOWN='true'
```

See also [Blink](#%EF%B8%8F-blink).

## 🎨 Draw

Create artwork and show it off on the display.

Use the [Web app](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-web-app) to get started.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_DRAW='true'
```

## 📊 Equalizer

Inspired by the original *IKEA Frekvens* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the equalizer, just like on a stock *IKEA Frekvens* device.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_EQUALIZER='true'
```

## 🙂 Face

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a minimalist face that looks around, blinks and changes expression.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_FACE='true'
```

## 🪨 Falling sand

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: grains of sand drop from a wandering point, pile up and slide sideways, then drain out once the pile reaches the top.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_FALLINGSAND='true'
```

## 🎆 Firework

Happy new year.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_FIREWORK='true'
```

## 🐦 Flappy bird

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: self-playing Flappy Bird, the bird predicts its trajectory and flaps through the gaps.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_FLAPPYBIRD='true'
```

## 🪰 Flies

Multiplayer fly-simulator.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_FLIES='true'
```

## 🌱 Game of life

Inspired by Conway’s *Game of Life*. Comes with an optional clock.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the game.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_GAMEOFLIFE='true'
```

## ✨ Glitter

Inspired by snow on old analog TVs.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause glittering.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_GLITTER='true'
```

## 🌡️ Home thermometer

Smart-home integration.

Display the current indoor and outdoor temperatures.

Integrated into [Home Assistant](https://github.com/VIPnytt/Frekvens/wiki/Extensions#home-assistant).

API payload example:

```json
{
    "indoor": 21,
    "outdoor": 9
}
```

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_HOMETHERMOMETER='true'
```

## 🍂 Leaf fall

Autumn-inspired.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause spawning of new leaves.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_LEAFFALL='true'
```

## ☰ Lines

Inspired by the original *IKEA Obegränsad* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the lines, just like on a stock *IKEA Frekvens* device.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_LINES='true'
```

See also [Scan](#-scan).

## 🟩 Matrix rain

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: falling columns with bright heads and fading trails.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_MATRIXRAIN='true'
```

## 🔵 Metaballs

Inspired by organic, blob-like shapes.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the movement of the shapes.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_METABALLS='true'
```

## 🌠 Meteor shower

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: meteors streaking diagonally across the display with short tails.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_METEORSHOWER='true'
```

## 🔇 Noise

Inspired by the original *IKEA Obegränsad* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the noise.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_NOISE='true'
```

## 🏓 Ping-pong

Inspired by the classic ping-pong game. Comes with an optional clock.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_PINGPONG='true'
```

## 🔢 Pixel sequence

For tech enthusiasts, it iterates through the pixels one by one, following the SPI bus addressing order.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the sequence.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_PIXELSEQUENCE='true'
```

## 🏓 Pong clock

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: pong that tells the time: the paddles let the ball through to score each hour and minute change, then the digits update.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_PONGCLOCK='true'
```

## 📡 Radar

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a rotating radar sweep with range rings and fading blips.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_RADAR='true'
```

## ☔ Rain

Inspired by the original *IKEA Obegränsad* mode.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_RAIN='true'
```

## 💍 Ring

Inspired by the original *IKEA Frekvens* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the ring, just like on a stock *IKEA Frekvens* device.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_RING='true'
```

See also [Circle](#-circle).

## 🔍 Scan

Inspired by the original *IKEA Obegränsad* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the scan.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SCAN='true'
```

See also [Lines](#-lines).

## 📺 Scanlines

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a horizontal scanline bouncing between the top and bottom rows.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SCANLINES='true'
```

## 🐍 Snake

Inspired by the classic snake-game. Comes with an optional clock.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SNAKE='true'
```

## 👾 Space invaders

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: self-playing Space Invaders, the ship tracks the nearest column of enemies and shoots them down wave after wave.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SPACEINVADERS='true'
```

## ✨ Sparkle field

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: random pixels flashing and fading out.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SPARKLEFIELD='true'
```

## 🌀 Spiral

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a rotating spiral arm that breathes in and out.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SPIRAL='true'
```

## 🔦 Spotlight

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: a soft spot of light drifting around and bouncing off the edges.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_SPOTLIGHT='true'
```

## ⭐ Stars

Inspired by the original *IKEA Obegränsad* mode.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause spawning of new stars.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_STARS='true'
```

## 💻 Stream

Provides real-time pixel and lighting data over Wi-Fi, allowing the display to be driven live from external software and controllers.

Multiple widely used streaming protocols are supported:

- Art-Net – Real-time lighting control data over UDP/IP.
- Distributed Display Protocol (DDP) – Efficient pixel-oriented data streaming.
- E1.31 (sACN) – Network-based DMX512 data streaming.

Endpoint examples:

- `frekvens.local:4048`
- `frekvens.local:5568`
- `frekvens.local:6454`

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_STREAM='true'
```

## 🧱 Tetris

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: self-playing Tetris, an AI picks the best rotation and column for every piece.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_TETRIS='true'
```

## 🅰️ Ticker

Rolling text.

Use the [Web app](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-web-app) to set a message, or automate it via [Home Assistant](https://github.com/VIPnytt/Frekvens/wiki/Extensions#-home-assistant).

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the rolling text.

API payload example:

```json
{
    "font": "Mini",
    "message": "Hello world!"
}
```

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_TICKER='true'
```

See also [Message](https://github.com/VIPnytt/Frekvens/wiki/Extensions#message).

## 🏍️ Tron

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: two light cycles steer by open space until one crashes, then both trails rewind.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_TRON='true'
```

## 🌊 Wave

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: three overlapping sine waves rendered as shifting brightness.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_WAVE='true'
```

## 📶 Wave bars

Inspired by the [ikea-led-obegraensad](https://github.com/ph1p/ikea-led-obegraensad) plugin of the same name: bottom-aligned bars whose heights follow a moving sine wave.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_WAVEBARS='true'
```

## 🔊 Waveform

Inspired by the two original *IKEA Frekvens* modes.

> [!TIP]
> The [Microphone](https://github.com/VIPnytt/Frekvens/wiki/Extensions#%EF%B8%8F-microphone) extension can be used to play/pause the waves, just like on a stock *IKEA Frekvens* device.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_WAVEFORM='true'
```

## ⛅ Weather

Providers available:

- [Open-Meteo](https://github.com/VIPnytt/Frekvens/wiki/Weather#-open-meteo)
- [Wttr.in](https://github.com/VIPnytt/Frekvens/wiki/Weather#-wttrin)
- [Yr](https://github.com/VIPnytt/Frekvens/wiki/Weather#-yr)

Configure in [secrets.h](https://github.com/VIPnytt/Frekvens/blob/main/firmware/include/config/secrets.h):

```h
#define LATITUDE "0.000"  // °
#define LONGITUDE "0.000" // °
```

To find your coordinates, use any preferred map or coordinate service, such as [LatLong.net](https://www.latlong.net/my-location-latitude-longitude), [GPS-Coordinates.net](https://www.gps-coordinates.net/my-location) or [GPS-Coordinates.org](https://gps-coordinates.org/my-location.php).

> [!NOTE]
> Coordinates with 3-4 decimal places are usually more than accurate enough for weather data.

Configure in [.env](https://github.com/VIPnytt/Frekvens/blob/main/.env):

```ini
MODE_WEATHER='true'
```
