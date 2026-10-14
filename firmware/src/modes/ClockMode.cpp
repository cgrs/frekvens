#if MODE_CLOCK

#include "modes/ClockMode.h"

#include "handlers/TextHandler.h"
#include "services/DisplayService.h"
#include "services/FontsService.h" // NOLINT(misc-include-cleaner)

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_CLOCK) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 16U, __STRING(MODE_CLOCK) " is not compatible with this device's display size.");

/**
 * @brief Marks the clock display for redraw.
 */
void ClockMode::begin() { pending = true; }

/**
 * @brief Updates the clock display and optional second indicator using the local time.
 */
void ClockMode::handle()
{
    if (getLocalTime(&local))
    {
        if (minute != local.tm_min || hour != local.tm_hour || pending)
        {
            hour = local.tm_hour;
            minute = local.tm_min;
#if CLOCK_12H
            const int hour{(local.tm_hour + 11) % 12 + 1};
#endif // CLOCK_12H
            const std::unique_ptr<const FontModule> fontModule{Fonts.get(fontNames[font])};
            const TextHandler hh1(std::to_string(hour / 10), *fontModule);
            const TextHandler hh2(std::to_string(hour % 10), *fontModule);
            const TextHandler mm1(std::to_string(minute / 10), *fontModule);
            const TextHandler mm2(std::to_string(minute % 10), *fontModule);
            const uint8_t fontWidth{max({hh1.getWidth(), hh2.getWidth(), mm1.getWidth(), mm2.getWidth()})};
            strikethrough = mm2.getHeight() > 6U;
            Display.fillFrame(0U);
            hh1.draw(static_cast<int16_t>((GRID_COLUMNS / 2U) - 1U - fontWidth + ((fontWidth - hh1.getWidth()) / 2U)),
                     static_cast<int8_t>((GRID_ROWS / 2U) - 1U - hh1.getHeight()));
            hh2.draw(static_cast<int16_t>((GRID_COLUMNS / 2U) + 1U + ((fontWidth - hh2.getWidth()) / 2U)),
                     static_cast<int8_t>((GRID_ROWS / 2U) - 1U - hh2.getHeight()));
            mm1.draw(static_cast<int16_t>((GRID_COLUMNS / 2U) - 1U - fontWidth + ((fontWidth - mm1.getWidth()) / 2U)),
                     static_cast<int8_t>((GRID_COLUMNS / 2U) + (strikethrough ? 1U : 0U)));
            mm2.draw(static_cast<int16_t>((GRID_COLUMNS / 2U) + 1U + ((fontWidth - mm2.getWidth()) / 2U)),
                     static_cast<int8_t>((GRID_COLUMNS / 2U) + (strikethrough ? 1U : 0U)));
            pending = false;
        }
        if (ticking && second != local.tm_sec)
        {
            drawTicker(0U);
            second = static_cast<uint8_t>(local.tm_sec);
            drawTicker(INT8_MAX);
        }
    }
}

void ClockMode::drawTicker(uint8_t brightness) const
{
    if (strikethrough)
    {
        Display.setPixel((GRID_COLUMNS / 2U) - (60U / 4U / 2U) - 1U + ((second + 2U) / 4U),
                         (static_cast<unsigned>(second) & 1U) == 0U ? (GRID_ROWS / 2U) - 1U : GRID_ROWS / 2U,
                         brightness);
    }
    else if (second < 8U)
    {
        Display.setPixel((GRID_COLUMNS / 2U) + second, (GRID_ROWS / 2U) - 8U, brightness);
    }
    else if (second < 8U + 15U)
    {
        Display.setPixel((GRID_COLUMNS / 2U) + 7U, (GRID_ROWS / 2U) - 15U + second, brightness);
    }
    else if (second < 8U + 30U)
    {
        Display.setPixel((GRID_COLUMNS / 2U) + 29U - second, (GRID_ROWS / 2U) + 7U, brightness);
    }
    else if (second < 8U + 45U)
    {
        Display.setPixel((GRID_COLUMNS / 2U) - 8U, (GRID_ROWS / 2U) + 44U - second, brightness);
    }
    else
    {
        Display.setPixel((GRID_COLUMNS / 2U) - 60U + second, (GRID_ROWS / 2U) - 8U, brightness);
    }
}

/**
 * @brief Gets the value of an option.
 *
 * @param index Index into the declared options.
 * @return The font index or whether the second indicator is shown.
 */
uint8_t ClockMode::getOption(size_t index) const { return index == 0U ? font : static_cast<uint8_t>(ticking); }

/**
 * @brief Sets the font or the second indicator and marks the display for redraw.
 *
 * @param index Index into the declared options.
 * @param value Font index or whether the second indicator is shown.
 */
void ClockMode::setOption(size_t index, uint8_t value)
{
    if (index == 0U)
    {
        font = value;
    }
    else
    {
        ticking = value != 0U;
    }
    pending = true;
}

#endif // MODE_CLOCK
