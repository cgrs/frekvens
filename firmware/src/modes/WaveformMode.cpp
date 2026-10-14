#if MODE_WAVEFORM

#include "modes/WaveformMode.h"

#include "handlers/BitmapHandler.h" // NOLINT(misc-include-cleaner)
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"

static_assert(GRID_COLUMNS == 16U, __STRING(MODE_WAVEFORM) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 12U, __STRING(MODE_WAVEFORM) " is not compatible with this device's display size.");

/**
 * @brief Updates the display with a waveform or centered row at regular intervals.
 *
 * When microphone support is enabled, draws a waveform for triggered input;
 * otherwise, fills the center row. Without microphone support, always draws a
 * waveform.
 */
void WaveformMode::handle()
{
    if (millis() - lastMillis > (0b1U << 9U))
    {
        Display.fillFrame(0U);
#if EXTENSION_MICROPHONE
        if (Extensions.Microphone().isTriggered())
        {
            draw();
        }
        else
        {
            Display.fillRow(GRID_ROWS / 2U, UINT8_MAX);
        }
#else
        draw();
#endif // EXTENSION_MICROPHONE
        lastMillis = millis();
    }
}

void WaveformMode::draw()
{
    const std::span<const std::span<const uint16_t>> &_wave{waves[static_cast<size_t>(wave)]};
    BitmapHandler(_wave[random(_wave.size())]).draw();
}

/**
 * @brief Gets the selected wave.
 *
 * @param index Index into the declared options.
 * @return Index of the selected wave in `waveNames`.
 */
uint8_t WaveformMode::getOption(size_t index) const { return static_cast<uint8_t>(wave); }

/**
 * @brief Selects the wave.
 *
 * @param index Index into the declared options.
 * @param value Index of the wave in `waveNames`.
 */
void WaveformMode::setOption(size_t index, uint8_t value) { wave = static_cast<Wave>(value); }

#endif // MODE_WAVEFORM
