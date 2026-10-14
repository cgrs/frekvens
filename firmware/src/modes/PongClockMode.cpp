#if MODE_PONGCLOCK

#include "modes/PongClockMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_PONGCLOCK) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 12U, __STRING(MODE_PONGCLOCK) " is not compatible with this device's display size.");

/**
 * @brief Resets the game state and clears the display until the time is available.
 */
void PongClockMode::begin()
{
    shownHour = 24;
    shownMinute = 60;
    scoreHour = false;
    scoreMinute = false;
    celebrating = false;
    Display.fillFrame(0U);
}

/**
 * @brief Plays the ping-pong game, where the time change is scored by a missed ball.
 *
 * The clock digits update only after the paddle on the matching side (hours left, minutes right) lets the ball pass.
 */
void PongClockMode::handle()
{
    if (!getLocalTime(&local))
    {
        return;
    }
    if (shownHour == 24)
    {
        shownHour = local.tm_hour;
        shownMinute = local.tm_min;
        drawDigits();
        launch();
        drawField();
        lastStep = millis();
        return;
    }
    if (celebrating)
    {
        if (millis() - lastBlink >= stepInterval)
        {
            ballVisible = !ballVisible;
            lastBlink = millis();
            drawField();
        }
        if (millis() >= celebrationEnd)
        {
            shownHour = local.tm_hour;
            shownMinute = local.tm_min;
            scoreHour = false;
            scoreMinute = false;
            Display.fillFrame(0U);
            drawDigits();
            launch();
            drawField();
            lastStep = millis();
        }
        return;
    }
    if (!scoreHour && !scoreMinute)
    {
        if (local.tm_hour != shownHour)
        {
            scoreHour = true;
            plan();
        }
        else if (local.tm_min != shownMinute)
        {
            scoreMinute = true;
            plan();
        }
    }
    if (millis() - lastStep >= stepInterval)
    {
        lastStep = millis();
        step();
        drawField();
    }
}

/**
 * @brief Moves the ball one row, reflecting it off the top and bottom of the playfield.
 *
 * @param row Ball row inside the playfield.
 * @param velocity Rows moved per step, inverted on reflection.
 */
void PongClockMode::advance(float &row, float &velocity)
{
    row += velocity;
    if (row < 0.0F)
    {
        row = -row;
        velocity = -velocity;
    }
    else if (row > fieldHeight - 1.0F)
    {
        row = 2.0F * (fieldHeight - 1.0F) - row;
        velocity = -velocity;
    }
}

/**
 * @brief Renders a 3×5 bitmap in the clock area.
 *
 * @param bitmap 15-bit bitmap containing the glyph pattern.
 * @param x Horizontal display position.
 */
void PongClockMode::drawDigit(uint16_t bitmap, uint8_t x) const
{
    for (uint8_t _x{0U}; _x < 3U; ++_x)
    {
        for (uint8_t _y{0U}; _y < 5U; ++_y)
        {
            Display.setPixel(static_cast<uint8_t>(x + _x),
                             _y,
                             ((bitmap >> (14U - ((3U * _y) + _x))) & 0b1U) != 0U ? INT8_MAX : 0U);
        }
    }
}

/**
 * @brief Renders the shown time, using the 12-hour format when configured.
 */
void PongClockMode::drawDigits() const
{
#if CLOCK_12H
    const int hour{(shownHour + 11) % 12 + 1};
#else
    const int hour{shownHour};
#endif // CLOCK_12H
    drawDigit(digits[hour / 10], (GRID_COLUMNS / 2U) - 8U);
    drawDigit(digits[hour % 10], (GRID_COLUMNS / 2U) - 4U);
    drawDigit(digits[shownMinute / 10], (GRID_COLUMNS / 2U) + 1U);
    drawDigit(digits[shownMinute % 10], (GRID_COLUMNS / 2U) + 5U);
}

/**
 * @brief Redraws the playfield with both paddles and the ball.
 */
void PongClockMode::drawField() const
{
    Display.fillRows(fieldTop, fieldHeight, 0U);
    for (int8_t offset{-1}; offset <= 1; ++offset)
    {
        Display.setPixel(0U, static_cast<uint8_t>(fieldTop + paddleLeft + offset), UINT8_MAX);
        Display.setPixel(GRID_COLUMNS - 1U, static_cast<uint8_t>(fieldTop + paddleRight + offset), UINT8_MAX);
    }
    if (ballVisible)
    {
        Display.setPixel(ballColumn, static_cast<uint8_t>(fieldTop + lroundf(ballRow)), UINT8_MAX);
    }
}

/**
 * @brief Serves the ball from the right side towards the left paddle.
 */
void PongClockMode::launch()
{
    celebrating = false;
    ballVisible = true;
    direction = -1;
    ballColumn = GRID_COLUMNS - 2U;
    ballRow = (fieldHeight - 1U) / 2.0F;
    ballVelocity = static_cast<float>(random(-60, 61)) / 100.0F;
    paddleLeft = paddleRight = targetLeft = targetRight = fieldHeight / 2U;
    plan();
}

/**
 * @brief Sets the target of the paddle the ball is heading to.
 *
 * The paddle meets the ball, unless its side has to score the time change, then it stays at least 2 rows away.
 */
void PongClockMode::plan()
{
    const uint8_t predicted{predictRow()};
    const bool towardsRight{direction > 0};
    uint8_t target{0U};
    if (towardsRight ? scoreMinute : scoreHour)
    {
        const int farTop{static_cast<int>(predicted) - 1};
        const int farBottom{static_cast<int>(fieldHeight) - 2 - static_cast<int>(predicted)};
        target = static_cast<uint8_t>(farTop > farBottom ? 1 : fieldHeight - 2);
    }
    else
    {
        target = std::min<uint8_t>(std::max<uint8_t>(predicted, 1U), fieldHeight - 2U);
    }
    (towardsRight ? targetRight : targetLeft) = target;
}

/**
 * @brief Predicts the row where the ball reaches the paddle column it is heading to.
 *
 * @return Row inside the playfield.
 */
uint8_t PongClockMode::predictRow() const
{
    float row{ballRow};
    float velocity{ballVelocity};
    const uint8_t distance{static_cast<uint8_t>(direction > 0 ? GRID_COLUMNS - 1U - ballColumn : ballColumn)};
    for (uint8_t n{0U}; n < distance; ++n)
    {
        advance(row, velocity);
    }
    return static_cast<uint8_t>(std::min<long>(std::max<long>(lroundf(row), 0L), fieldHeight - 1L));
}

/**
 * @brief Moves the paddles and the ball one step, and resolves the ball reaching a paddle column.
 */
void PongClockMode::step()
{
    if (paddleLeft != targetLeft)
    {
        paddleLeft = static_cast<uint8_t>(paddleLeft + (targetLeft > paddleLeft ? 1 : -1));
    }
    if (paddleRight != targetRight)
    {
        paddleRight = static_cast<uint8_t>(paddleRight + (targetRight > paddleRight ? 1 : -1));
    }
    advance(ballRow, ballVelocity);
    const int next{static_cast<int>(ballColumn) + direction};
    if (next > 0 && next < static_cast<int>(GRID_COLUMNS) - 1)
    {
        ballColumn = static_cast<uint8_t>(next);
        return;
    }
    const int paddle{next <= 0 ? paddleLeft : paddleRight};
    if (std::abs(static_cast<int>(lroundf(ballRow)) - paddle) <= 1)
    {
        direction = static_cast<int8_t>(-direction);
        ballVelocity = static_cast<float>(random(-60, 61)) / 100.0F;
        plan();
    }
    else
    {
        ballColumn = static_cast<uint8_t>(next);
        celebrating = true;
        celebrationEnd = millis() + celebrationDuration;
        lastBlink = millis();
    }
}

#endif // MODE_PONGCLOCK
