#if MODE_PINGPONG

#include "modes/PingPongMode.h"

#include "fonts/MiniFont.h"       // NOLINT(misc-include-cleaner)
#include "handlers/TextHandler.h" // NOLINT(misc-include-cleaner)
#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_PINGPONG) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 9U, __STRING(MODE_PINGPONG) " is not compatible with this device's display size.");

/**
 * @brief Initializes the ping-pong display animation.
 *
 * Initializes the paddles and ball, and draws them in the layout appropriate for the selected mode.
 */
void PingPongMode::begin()
{
    Display.fillFrame(0U);
#if GRID_COLUMNS == GRID_ROWS
    const uint8_t _paddle{static_cast<uint8_t>(random(clock == nullptr ? 0 : 5, GRID_COLUMNS - 3U))};
#else
    const uint8_t _paddle{random(clock == nullptr ? 0 : 5, clock ? GRID_ROWS - 3U : GRID_COLUMNS - 3U)};
#endif // GRID_COLUMNS == GRID_ROWS
    for (uint8_t idx{0U}; idx < 3U; ++idx)
    {
        paddleA.push_back(_paddle + idx);
        paddleB.push_back(_paddle + idx);
    }
    if (clock == nullptr)
    {
        Display.drawLineHorizontal(_paddle, 3U, 0U, UINT8_MAX);
        Display.drawLineHorizontal(_paddle, 3U, GRID_ROWS - 1U, UINT8_MAX);
    }
    else
    {
        Display.drawLineVertical(0U, _paddle, _paddle + 2U, INT8_MAX);
        Display.drawLineVertical(GRID_COLUMNS - 1U, _paddle, _paddle + 2U, INT8_MAX);
    }
    xDec = x = clock == nullptr ? _paddle + 1U : GRID_COLUMNS - 2U;
    yDec = y = clock == nullptr ? GRID_ROWS - 2U : _paddle + 1U;
    deg = static_cast<uint16_t>(random(clock == nullptr ? 60 : 150, clock == nullptr ? 121 : 211)); // ±30°
    Display.setPixel(x, y, clock == nullptr ? UINT8_MAX : INT8_MAX);
    began = true;
}

/**
 * @brief Advances the ping-pong animation and adjusts paddle positions.
 *
 * Updates the ball position and direction, reflects it at the relevant display
 * boundaries, and moves paddles toward the ball when interception is possible.
 */
void PingPongMode::handle()
{
    if (clock != nullptr)
    {
        clock->handle();
        if (xDec <= 1 && deg >= 90U && deg < 270U)
        {
            // Left
            deg = (random(45, 136) + 270U) % 360U; // ±45°
        }
        else if (xDec >= GRID_COLUMNS - 2U && (deg < 90U || deg >= 270U))
        {
            // Right
            deg = static_cast<uint16_t>(random(135, 226)); // ±45°
        }
    }
    else
    {
        if (yDec >= GRID_ROWS - 2U && deg >= 180U)
        {
            // Bottom
            deg = static_cast<uint16_t>(random(45, 136)); // ±45°
        }
        else if (yDec <= 1U && deg < 180U)
        {
            // Top
            deg = static_cast<uint16_t>(random(225, 316)); // ±45°
        }
    }
    if ((clock == nullptr && xDec <= 0U) || xDec >= GRID_COLUMNS - 1U)
    {
        // Left/right
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
    }
    if ((clock != nullptr && yDec <= 5U) || yDec >= GRID_ROWS - 1U)
    {
        // Top/bottom
        deg = 360U - deg; // Invert Y
    }
    Display.setPixel(x, y, 0U);
    xDec += cosf(static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)) * speed;
    yDec -= sinf(static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)) * speed;
    x = lroundf(xDec);
    y = lroundf(yDec);
    Display.setPixel(x, y, clock == nullptr ? UINT8_MAX : INT8_MAX);
    const float aRad{
        atanf(clock == nullptr ? (yDec - 1U) / abs(paddleA[1U] - xDec) : (xDec - 1U) / abs(paddleA[1U] - yDec))};
    const float bRad{atanf(clock == nullptr ? (GRID_ROWS - 2U - yDec) / abs(paddleB[1U] - xDec)
                                            : (GRID_COLUMNS - 2U - xDec) / abs(paddleB[1U] - yDec))};
    if (clock == nullptr && xDec < paddleB.front() && bRad < 1U && paddleB.front() != 0U)
    {
        // Bottom: left
        Display.setPixel(paddleB.back(), GRID_ROWS - 1U, 0U);
        paddleB.pop_back();
        paddleB.push_front(paddleB.front() - 1U);
        Display.setPixel(paddleB.front(), GRID_ROWS - 1U, UINT8_MAX);
    }
    else if (clock == nullptr && xDec > paddleB.back() && bRad < 1U && paddleB.back() < GRID_COLUMNS - 1U)
    {
        // Bottom: right
        Display.setPixel(paddleB.front(), GRID_ROWS - 1U, 0U);
        paddleB.pop_front();
        paddleB.push_back(paddleB.back() + 1U);
        Display.setPixel(paddleB.back(), GRID_ROWS - 1U, UINT8_MAX);
    }
    else if (clock != nullptr && yDec > paddleA.back() && aRad < 1U && paddleA.back() < GRID_ROWS - 1U)
    {
        // Left: down
        Display.setPixel(0U, paddleA.front(), 0U);
        paddleA.pop_front();
        paddleA.push_back(paddleA.back() + 1U);
        Display.setPixel(0U, paddleA.back(), INT8_MAX);
    }
    else if (clock != nullptr && yDec < paddleA.front() && aRad < 1U && paddleA.front() > 5U)
    {
        // Left: up
        Display.setPixel(0U, paddleA.back(), 0U);
        paddleA.pop_back();
        paddleA.push_front(paddleA.front() - 1U);
        Display.setPixel(0U, paddleA.front(), INT8_MAX);
    }
    else if (clock != nullptr && yDec > paddleB.back() && bRad < 1U && paddleB.back() < GRID_ROWS - 1U)
    {
        // Right: down
        Display.setPixel(GRID_COLUMNS - 1U, paddleB.front(), 0U);
        paddleB.pop_front();
        paddleB.push_back(paddleB.back() + 1U);
        Display.setPixel(GRID_COLUMNS - 1U, paddleB.back(), INT8_MAX);
    }
    else if (clock != nullptr && yDec < paddleB.front() && bRad < 1U && paddleB.front() > 5U)
    {
        // Right: up
        Display.setPixel(GRID_COLUMNS - 1U, paddleB.back(), 0U);
        paddleB.pop_back();
        paddleB.push_front(paddleB.front() - 1U);
        Display.setPixel(GRID_COLUMNS - 1U, paddleB.front(), INT8_MAX);
    }
    else if (clock == nullptr && xDec < paddleA.front() && aRad < 1U && paddleA.front() != 0U)
    {
        // Top: left
        Display.setPixel(paddleA.back(), 0U, 0U);
        paddleA.pop_back();
        paddleA.push_front(paddleA.front() - 1U);
        Display.setPixel(paddleA.front(), 0U, UINT8_MAX);
    }
    else if (clock == nullptr && xDec > paddleA.back() && aRad < 1U && paddleA.back() < GRID_COLUMNS - 1U)
    {
        // Top: right
        Display.setPixel(paddleA.front(), 0U, 0U);
        paddleA.pop_front();
        paddleA.push_back(paddleA.back() + 1U);
        Display.setPixel(paddleA.back(), 0U, UINT8_MAX);
    }
}

/**
 * @brief Gets the value of an option.
 *
 * @param index Index into the declared options.
 * @return Whether the clock is enabled.
 */
uint8_t PingPongMode::getOption(size_t index) const { return static_cast<uint8_t>(clock != nullptr); }

/**
 * @brief Enables or disables the clock.
 *
 * Before `begin()` it only creates or drops the clock handler; once running it also clears the clock area and
 * reshapes the paddles.
 *
 * @param index Index into the declared options.
 * @param value Whether the clock should be enabled.
 */
void PingPongMode::setOption(size_t index, uint8_t value)
{
    if (value != 0U)
    {
        if (clock == nullptr)
        {
            clock = std::make_unique<ClockHandler>();
            if (began)
            {
                yDec = y = 6U;
                clock->clear();
                for (const uint8_t x : paddleB)
                {
                    Display.setPixel(x, GRID_ROWS - 1U, 0U);
                }
            }
        }
    }
    else if (clock != nullptr)
    {
        if (!began)
        {
            clock.reset();
            return;
        }
        clock->clear();
        clock.reset();
        for (const uint8_t y : paddleA)
        {
            Display.setPixel(0U, y, 0U);
        }
        for (const uint8_t y : paddleB)
        {
            Display.setPixel(GRID_COLUMNS - 1U, y, 0U);
        }
    }
}

#endif // MODE_PINGPONG
