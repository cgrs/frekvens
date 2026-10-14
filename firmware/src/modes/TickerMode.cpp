#if MODE_TICKER

#include "modes/TickerMode.h"

#include "fonts/SmallFont.h" // NOLINT(misc-include-cleaner)
#include "modules/Trigger.h"
#include "services/DisplayService.h"
#include "services/FontsService.h" // NOLINT(misc-include-cleaner)

/**
 * @brief Prepares the ticker for rendering with the selected font.
 */
void TickerMode::begin()
{
    if (!font)
    {
        font = Fonts.get(FontsService::names[fontIndex]);
    }
    pending = true;
}

/**
 * @brief Updates and renders the scrolling ticker text.
 *
 * Initializes pending text, advances its position at timed intervals, and
 * restarts the scroll cycle after the text leaves the display. When microphone
 * support is enabled, a microphone trigger is required to begin each cycle.
 */
void TickerMode::handle()
{
    if (pending)
    {
        text = std::make_unique<TextHandler>(message, *font);
        offsetX = GRID_COLUMNS;
        offsetY = (GRID_ROWS - text->getHeight()) / 2U;
        width = text->getWidth();
        transmitOptions();
        pending = false;
    }
    else if (text && millis() - lastMillis > INT8_MAX)
    {
        if (width + offsetX < 0)
        {
            offsetX = GRID_COLUMNS;
        }
        lastMillis = millis();
        if (offsetX == GRID_COLUMNS && !Trigger::active())
        {
            return;
        }
        Display.fillFrame(0U);
        text->draw(offsetX, offsetY);
        --offsetX;
    }
}

/**
 * @brief Gets the font index of an option.
 *
 * @param index Index into the declared options.
 */
uint8_t TickerMode::getOption(size_t index) const { return fontIndex; }

/**
 * @brief Selects the font.
 *
 * @param index Index into the declared options.
 * @param value Index into the available fonts.
 */
void TickerMode::setOption(size_t index, uint8_t value)
{
    fontIndex = value;
    font = Fonts.get(FontsService::names[fontIndex]);
    pending = true;
}

/**
 * @brief Gets the message.
 *
 * @param index Index into the declared options.
 */
std::string TickerMode::getText(size_t index) const { return message; }

/**
 * @brief Sets the message unless it is empty.
 *
 * @param index Index into the declared options.
 * @param value New message.
 * @return Whether the message was accepted.
 */
bool TickerMode::setText(size_t index, std::string_view value)
{
    if (value.empty())
    {
        return false;
    }
    message = value;
    pending = true;
    return true;
}

/**
 * @brief Releases the active text handler.
 */
void TickerMode::end() { text.reset(); }

#endif // MODE_TICKER
