#if MODE_FLAPPYBIRD

#include "modes/FlappyBirdMode.h"

#include "services/DisplayService.h"

#include <algorithm>

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_FLAPPYBIRD) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 16U, __STRING(MODE_FLAPPYBIRD) " is not compatible with this device's display size.");

/**
 * @brief Clears the display and starts a new game.
 */
void FlappyBirdMode::begin()
{
    Display.fillFrame(0U);
    resetGame();
}

/**
 * @brief Advances the self-playing game and renders the scene when a step has elapsed.
 *
 * Moves the pipes and the bird at their own intervals while running, then blinks, drops the bird
 * and pauses after a crash before restarting.
 */
void FlappyBirdMode::handle()
{
    switch (stage)
    {
    case Stage::RUNNING:
        if (millis() - lastPipes >= pipeInterval)
        {
            lastPipes = millis();
            tickPipes();
        }
        if (millis() - lastPhysics >= physicsInterval)
        {
            lastPhysics = millis();
            tickPhysics();
        }
        break;
    case Stage::CRASH_BLINK:
        if (millis() - lastSequence < blinkInterval)
        {
            return;
        }
        lastSequence = millis();
        blinkVisible = !blinkVisible;
        ++blinkToggles;
        if (blinkToggles >= blinkTogglesMax)
        {
            blinkVisible = true;
            blinkToggles = 0U;
            stage = Stage::CRASH_FALL;
        }
        break;
    case Stage::CRASH_FALL:
        if (millis() - lastPhysics < physicsInterval)
        {
            return;
        }
        lastPhysics = millis();
        vy = clampVy(vy + gravity);
        by += vy;
        if (by >= floorY)
        {
            by = floorY;
            stage = Stage::CRASH_PAUSE;
            lastSequence = millis();
        }
        break;
    case Stage::CRASH_PAUSE:
        if (millis() - lastSequence < crashPause)
        {
            return;
        }
        resetGame();
        break;
    }
    render();
}

/**
 * @brief Limits the vertical velocity to its allowed range.
 */
float FlappyBirdMode::clampVy(float value) { return std::clamp(value, vyMin, vyMax); }

/**
 * @brief Checks whether a row of a pipe is solid, as opposed to the gap.
 */
bool FlappyBirdMode::isSolidRow(const Pipe &pipe, int y)
{
    return y < static_cast<int>(pipe.gy) || y >= static_cast<int>(pipe.gy + gapHeight);
}

/**
 * @brief Adds a pipe at the right edge with a random gap position.
 */
void FlappyBirdMode::spawnPipe()
{
    if (pipeCount >= pipeMax)
    {
        return;
    }
    Pipe &pipe{pipes[pipeCount]};
    ++pipeCount;
    pipe.x = spawnX;
    pipe.gy = static_cast<uint8_t>(random(gapTopMin, gapTopMax + 1));
}

/**
 * @brief Scrolls the pipes one column left, drops those that left the display and spawns new ones.
 */
void FlappyBirdMode::tickPipes()
{
    for (uint8_t idx{0U}; idx < pipeCount; ++idx)
    {
        --pipes[idx].x;
    }
    uint8_t writeIndex{0U};
    for (uint8_t idx{0U}; idx < pipeCount; ++idx)
    {
        if (pipes[idx].x >= 0)
        {
            pipes[writeIndex++] = pipes[idx];
        }
    }
    pipeCount = writeIndex;
    if (pipeCount == 0U || pipes[pipeCount - 1U].x <= spawnThresholdX)
    {
        spawnPipe();
    }
}

/**
 * @brief Lets the AI decide on a flap, applies gravity and checks for crashes.
 */
void FlappyBirdMode::tickPhysics()
{
    updateAi();
    vy = clampVy(vy + gravity);
    by += vy;
    if (by < ceilingY)
    {
        by = ceilingY;
        if (vy < 0.0F)
        {
            vy = 0.0F;
        }
    }
    if (by > floorY)
    {
        by = floorY;
        startCrash();
        return;
    }
    if (birdHitsPipe())
    {
        startCrash();
    }
}

/**
 * @brief Finds the nearest pipe at or ahead of the bird.
 *
 * @return Pointer to the pipe, or `nullptr` when none is ahead.
 */
const FlappyBirdMode::Pipe *FlappyBirdMode::findControllingPipe() const
{
    const Pipe *controlling{nullptr};
    for (uint8_t idx{0U}; idx < pipeCount; ++idx)
    {
        const Pipe &pipe{pipes[idx]};
        if (static_cast<int>(pipe.x) < static_cast<int>(birdX))
        {
            continue;
        }
        if (controlling == nullptr || pipe.x < controlling->x)
        {
            controlling = &pipe;
        }
    }
    return controlling;
}

/**
 * @brief Decides whether to flap by predicting the bird's trajectory against the next pipe gap.
 */
void FlappyBirdMode::updateAi()
{
    float simY{by};
    float simV{vy};
    float projectedMinY{by};
    for (uint8_t tick{0U}; tick < predictTicks; ++tick)
    {
        simV = clampVy(simV + gravity);
        simY += simV;
        projectedMinY = std::min(projectedMinY, simY);
    }

    const Pipe *pipe{findControllingPipe()};
    const int dx{pipe == nullptr ? 0 : static_cast<int>(pipe->x) - static_cast<int>(birdX)};
    bool flap{false};

    if (pipe != nullptr && dx >= 0 && dx <= static_cast<int>(nearDxMax))
    {
        const float upper{static_cast<float>(pipe->gy) + bandUpperOffset};
        const float lower{static_cast<float>(pipe->gy) + bandLowerOffset};
        const bool inBand{by >= upper && by <= lower};
        const bool gapEntrySafe{by > lower && projectedMinY > upper};
        const bool floorRisk{simY > pipeFloorRiskY && by > upper + 1.0F};
        const bool risingIntoBand{vy < riseSuppressVy || (vy < 0.0F && by < upper + 1.0F)};
        const bool wantsFlap{inBand ? floorRisk : (gapEntrySafe || floorRisk)};
        flap = wantsFlap && !risingIntoBand;
    }
    else
    {
        const bool cruiseFall{by > cruiseFlapY || (by > cruiseHoldY && vy > cruiseFallVy)};
        const bool floorRisk{simY > cruiseFloorRiskY};
        flap = cruiseFall || floorRisk;
    }

    if (flap)
    {
        vy = flapVy;
    }
}

/**
 * @brief Checks whether the bird overlaps a solid part of a pipe.
 */
bool FlappyBirdMode::birdHitsPipe() const
{
    const int cell{static_cast<int>(by + 0.5F)};
    for (uint8_t idx{0U}; idx < pipeCount; ++idx)
    {
        const Pipe &pipe{pipes[idx]};
        if (static_cast<int>(pipe.x) == static_cast<int>(birdX) && isSolidRow(pipe, cell))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Starts the crash blink sequence.
 */
void FlappyBirdMode::startCrash()
{
    blinkVisible = true;
    blinkToggles = 0U;
    stage = Stage::CRASH_BLINK;
    lastSequence = millis();
}

/**
 * @brief Removes all pipes, resets the bird and restarts the timers.
 */
void FlappyBirdMode::resetGame()
{
    pipeCount = 0U;
    by = birdStartY;
    vy = 0.0F;
    blinkVisible = true;
    blinkToggles = 0U;
    lastPhysics = millis();
    lastPipes = millis();
    lastSequence = millis();
    stage = Stage::RUNNING;
}

/**
 * @brief Draws a pipe as two columns of pixels around its gap.
 */
void FlappyBirdMode::drawPipe(const Pipe &pipe)
{
    if (pipe.x < 0 || pipe.x >= static_cast<int8_t>(GRID_COLUMNS))
    {
        return;
    }
    const uint8_t gapBottom{static_cast<uint8_t>(std::min<uint8_t>(pipe.gy + gapHeight, GRID_ROWS))};
    for (uint8_t y{0U}; y < pipe.gy && y < GRID_ROWS; ++y)
    {
        Display.setPixel(static_cast<uint8_t>(pipe.x), y, brightnessHigh);
    }
    for (uint8_t y{gapBottom}; y < GRID_ROWS; ++y)
    {
        Display.setPixel(static_cast<uint8_t>(pipe.x), y, brightnessHigh);
    }
}

/**
 * @brief Draws the pipes and the bird, hiding them during the off phase of the crash blink.
 */
void FlappyBirdMode::render() const
{
    Display.fillFrame(0U);
    if (stage == Stage::CRASH_BLINK && !blinkVisible)
    {
        return;
    }
    for (uint8_t idx{0U}; idx < pipeCount; ++idx)
    {
        drawPipe(pipes[idx]);
    }
    Display.setPixel(birdX, static_cast<uint8_t>(std::min(by + 0.5F, floorY)), brightnessFull);
}

#endif // MODE_FLAPPYBIRD
