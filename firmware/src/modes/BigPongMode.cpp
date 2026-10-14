#if MODE_BIGPONG

#include "modes/BigPongMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <cmath>
#include <numbers>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_BIGPONG) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_BIGPONG) " is not compatible with this device's display size.");

/**
 * @brief Clears the display, centers the paddles and serves the ball.
 */
void BigPongMode::begin()
{
    Display.fillFrame(0U);
    topPaddleX = clampPaddle((GRID_COLUMNS - paddleWidth) / 2);
    bottomPaddleX = topPaddleX;
    topTargetX = topPaddleX;
    bottomTargetX = bottomPaddleX;
    serve(random(2) == 0 ? 1.0F : -1.0F);
    stage = Stage::RUNNING;
    lastMillis = millis();
}

/**
 * @brief Advances the self-playing match, the goal blink and the serve delay at their own intervals.
 */
void BigPongMode::handle()
{
    const unsigned long interval = stage == Stage::RUNNING      ? moveInterval
                                 : stage == Stage::GOAL_BLINK ? goalBlinkInterval
                                                              : serveDelay;
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    switch (stage)
    {
    case Stage::RUNNING:
        tickGame();
        break;
    case Stage::GOAL_BLINK:
        blinkVisible = !blinkVisible;
        ++blinkToggles;
        if (blinkToggles >= goalBlinkToggles)
        {
            stage = Stage::SERVE_DELAY;
        }
        break;
    case Stage::SERVE_DELAY:
        serve(serveDirY);
        stage = Stage::RUNNING;
        break;
    }
    render();
}

/**
 * @brief Limits a paddle position so the paddle stays on the display.
 */
int8_t BigPongMode::clampPaddle(int paddle)
{
    return static_cast<int8_t>(std::clamp(paddle, 0, static_cast<int>(GRID_COLUMNS - paddleWidth)));
}

/**
 * @brief Rolls a random horizontal aim error for a paddle, mostly 0 or 1 pixel.
 */
int8_t BigPongMode::rollAimError()
{
    int8_t magnitude{static_cast<int8_t>(random(aimErrorMax + 1))};
    if (random(100) < 20)
    {
        magnitude = static_cast<int8_t>(magnitude + random(1, 3));
    }
    return random(2) == 0 ? magnitude : static_cast<int8_t>(-magnitude);
}

/**
 * @brief Speeds the ball up, up to a cap.
 */
void BigPongMode::scaleBallSpeed()
{
    const float current{std::sqrt((vx * vx) + (vy * vy))};
    if (current <= 0.0F)
    {
        return;
    }
    const float speed{std::min(current * speedUp, baseSpeed * maxSpeedFactor)};
    const float scale{speed / current};
    vx *= scale;
    vy *= scale;
}

/**
 * @brief Reflects the ball off a paddle, deflecting it by the offset from the paddle center.
 */
void BigPongMode::bounceOffPaddle(bool top)
{
    const float centerX{static_cast<float>(top ? topPaddleX : bottomPaddleX) + ((paddleWidth - 1U) / 2.0F)};
    vx += (bx - centerX) * nudge;
    by = top ? -by : (2.0F * paddleBottomY) - by;
    vy = -vy;
    scaleBallSpeed();
    aimErrorTop = rollAimError();
    aimErrorBottom = rollAimError();
}

/**
 * @brief Places the ball at the center and launches it at a random angle.
 *
 * @param dirY Vertical direction, 1 toward the bottom paddle, -1 toward the top.
 */
void BigPongMode::serve(float dirY)
{
    bx = (GRID_COLUMNS - 1U) / 2.0F;
    by = (GRID_ROWS - 1U) / 2.0F;
    const float angle{static_cast<float>(random(serveAngleMaxDeg + 1)) * std::numbers::pi_v<float> / 180.0F};
    const float side{random(2) == 0 ? 1.0F : -1.0F};
    vx = side * baseSpeed * std::sin(angle);
    vy = dirY * baseSpeed * std::cos(angle);
    aimErrorTop = rollAimError();
    aimErrorBottom = rollAimError();
}

/**
 * @brief Starts the goal blink; the ball is served toward the player who just scored.
 */
void BigPongMode::startGoal(Player scorer)
{
    scoringPlayer = scorer;
    serveDirY = scorer == Player::TOP ? 1.0F : -1.0F;
    blinkVisible = true;
    blinkToggles = 0U;
    stage = Stage::GOAL_BLINK;
}

/**
 * @brief Moves the paddles and the ball, bouncing off walls and paddles, and scores a goal when the ball passes one.
 */
void BigPongMode::tickGame()
{
    updateAi();

    bx += vx;
    by += vy;

    if (bx < 0.0F)
    {
        bx = 0.0F;
        vx = -vx;
    }
    else if (bx > static_cast<float>(GRID_COLUMNS - 1U))
    {
        bx = static_cast<float>(GRID_COLUMNS - 1U);
        vx = -vx;
    }

    const int ballX{static_cast<int>(bx + 0.5F)};

    if (by <= static_cast<float>(paddleTopY) && vy < 0.0F && ballX >= topPaddleX &&
        ballX <= topPaddleX + static_cast<int>(paddleWidth) - 1)
    {
        bounceOffPaddle(true);
    }
    if (by >= static_cast<float>(paddleBottomY) && vy > 0.0F && ballX >= bottomPaddleX &&
        ballX <= bottomPaddleX + static_cast<int>(paddleWidth) - 1)
    {
        bounceOffPaddle(false);
    }

    if (by < 0.0F)
    {
        startGoal(Player::BOTTOM);
    }
    else if (by > static_cast<float>(paddleBottomY))
    {
        startGoal(Player::TOP);
    }
}

/**
 * @brief Predicts the column where the ball reaches a paddle row, with wall bounces.
 *
 * @param top true for the top paddle row, false for the bottom.
 */
int8_t BigPongMode::predictLanding(bool top) const
{
    const float targetY{top ? static_cast<float>(paddleTopY) : static_cast<float>(paddleBottomY)};
    // Steps exactly like tickGame() so the prediction stays the same for the whole flight
    float px{bx};
    float py{by};
    float pvx{vx};
    for (uint16_t step{0U}; step < predictMaxSteps; ++step)
    {
        if (top ? (py <= targetY) : (py >= targetY))
        {
            break;
        }
        px += pvx;
        py += vy;
        if (px < 0.0F)
        {
            px = 0.0F;
            pvx = -pvx;
        }
        else if (px > static_cast<float>(GRID_COLUMNS - 1U))
        {
            px = static_cast<float>(GRID_COLUMNS - 1U);
            pvx = -pvx;
        }
    }
    return clampPaddle(static_cast<int>(px + 0.5F));
}

/**
 * @brief Aims the paddle in the ball's path at the predicted landing, then moves both paddles one pixel.
 */
void BigPongMode::updateAi()
{
    if (vy < 0.0F)
    {
        const int target{predictLanding(true) - ((paddleWidth - 1) / 2)};
        topTargetX = clampPaddle(target + aimErrorTop);
    }
    else if (vy > 0.0F)
    {
        const int target{predictLanding(false) - ((paddleWidth - 1) / 2)};
        bottomTargetX = clampPaddle(target + aimErrorBottom);
    }

    if (topPaddleX < topTargetX)
    {
        ++topPaddleX;
    }
    else if (topPaddleX > topTargetX)
    {
        --topPaddleX;
    }
    if (bottomPaddleX < bottomTargetX)
    {
        ++bottomPaddleX;
    }
    else if (bottomPaddleX > bottomTargetX)
    {
        --bottomPaddleX;
    }
}

/**
 * @brief Draws a paddle row of fixed width.
 */
void BigPongMode::drawPaddleRow(int8_t x, uint8_t y)
{
    for (uint8_t idx{0U}; idx < paddleWidth; ++idx)
    {
        Display.setPixel(static_cast<uint8_t>(x + idx), y, brightnessFull);
    }
}

/**
 * @brief Draws the center line, both paddles and the ball, blinking the scorer's paddle and the ball on a goal.
 */
void BigPongMode::render() const
{
    Display.fillFrame(0U);

    for (uint8_t x{0U}; x < GRID_COLUMNS; x += 2U)
    {
        Display.setPixel(x, midLineY, brightnessFaint);
    }

    const bool blinkOff{stage == Stage::GOAL_BLINK && !blinkVisible};
    if (!blinkOff || scoringPlayer != Player::TOP)
    {
        drawPaddleRow(topPaddleX, paddleTopY);
    }
    if (!blinkOff || scoringPlayer != Player::BOTTOM)
    {
        drawPaddleRow(bottomPaddleX, paddleBottomY);
    }

    bool drawBall{false};
    uint8_t ballY{static_cast<uint8_t>(std::clamp(by + 0.5F, 0.0F, static_cast<float>(paddleBottomY)))};
    if (stage == Stage::RUNNING)
    {
        drawBall = true;
    }
    else if (stage == Stage::GOAL_BLINK && blinkVisible)
    {
        drawBall = true;
        ballY = scoringPlayer == Player::BOTTOM ? paddleTopY : paddleBottomY;
    }
    if (drawBall)
    {
        const uint8_t ballX{static_cast<uint8_t>(std::clamp(bx + 0.5F, 0.0F, static_cast<float>(GRID_COLUMNS - 1U)))};
        Display.setPixel(ballX, ballY, brightnessFull);
    }
}

#endif // MODE_BIGPONG
