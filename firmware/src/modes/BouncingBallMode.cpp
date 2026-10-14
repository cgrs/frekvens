#if MODE_BOUNCINGBALL

#include "modes/BouncingBallMode.h"

#include "services/DisplayService.h"

#include <cmath>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_BOUNCINGBALL) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_BOUNCINGBALL) " is not compatible with this device's display size.");

/**
 * @brief Clears the display, generates a level and drops the first ball.
 */
void BouncingBallMode::begin()
{
    Display.fillFrame(0U);
    obstacleBrightness = brightnessObstacle;
    generateLevel();
    spawnBall();
    stage = Stage::RUN;
    physicsMillis = fadeMillis = millis();
    render();
}

/**
 * @brief Runs the ball physics, then fades the obstacles out when the ball lands or times out.
 */
void BouncingBallMode::handle()
{
    switch (stage)
    {
    case Stage::RUN:
        if (millis() - physicsMillis >= physicsInterval)
        {
            physicsMillis = millis();
            tick();
            render();
        }
        if (stage == Stage::RUN && millis() - levelMillis > levelTimeout)
        {
            stage = Stage::END;
        }
        break;
    case Stage::END:
        if (millis() - fadeMillis >= fadeInterval)
        {
            fadeMillis = millis();
            obstacleBrightness = obstacleBrightness > fadeStep ? static_cast<uint8_t>(obstacleBrightness - fadeStep) : 0U;
            if (obstacleBrightness == 0U)
            {
                generateLevel();
                spawnBall();
                obstacleBrightness = brightnessObstacle;
                stage = Stage::RUN;
            }
            render();
        }
        break;
    }
}

/**
 * @brief Places single-cell and horizontal-segment obstacles at random, keeping them apart.
 */
void BouncingBallMode::generateLevel()
{
    obstacleCount = 0U;
    std::array<std::array<bool, GRID_COLUMNS>, GRID_ROWS> reserved{};
    const long target{6L + random(5L)};
    for (long i{0L}; i < target && obstacleCount < obstaclesMax; ++i)
    {
        const bool segment{random(2L) == 0L};
        for (uint8_t attempt{0U}; attempt < 20U; ++attempt)
        {
            const uint8_t length{static_cast<uint8_t>(segment ? random(3L, 6L) : 1L)};
            const uint8_t x{static_cast<uint8_t>(random(0L, static_cast<long>(GRID_COLUMNS) - length + 1L))};
            const uint8_t y{static_cast<uint8_t>(random(2L, static_cast<long>(GRID_ROWS) - 2L))};
            bool free{true};
            for (uint8_t c{0U}; c < length; ++c)
            {
                if (reserved[y][x + c])
                {
                    free = false;
                    break;
                }
            }
            if (!free)
            {
                continue;
            }
            for (int c{-1}; c <= static_cast<int>(length); ++c)
            {
                for (int dy{-1}; dy <= 1; ++dy)
                {
                    const int rx{static_cast<int>(x) + c};
                    const int ry{static_cast<int>(y) + dy};
                    if (rx >= 0 && rx < static_cast<int>(GRID_COLUMNS) && ry >= 0 && ry < static_cast<int>(GRID_ROWS))
                    {
                        reserved[ry][rx] = true;
                    }
                }
            }
            obstacles[obstacleCount++] = {x, y, length};
            break;
        }
    }
}

/**
 * @brief Drops a new ball from the top row with a random sideways speed.
 */
void BouncingBallMode::spawnBall()
{
    posX = static_cast<float>(random(1L, static_cast<long>(GRID_COLUMNS) - 1L));
    posY = 0.0F;
    velY = 0.0F;
    const float speed{static_cast<float>(random(18L, 41L)) / 100.0F};
    velX = random(2L) == 0L ? speed : -speed;
    levelMillis = millis();
}

/**
 * @brief Bounces the ball off an obstacle cell, with a little random jitter on the other axis.
 */
void BouncingBallMode::collide(uint8_t cellX, uint8_t cellY)
{
    const float left{static_cast<float>(cellX)};
    const float top{static_cast<float>(cellY)};
    const float nearestX{posX < left ? left : (posX > left + 1.0F ? left + 1.0F : posX)};
    const float nearestY{posY < top ? top : (posY > top + 1.0F ? top + 1.0F : posY)};
    const float dx{posX - nearestX};
    const float dy{posY - nearestY};
    if (dx * dx + dy * dy >= radius * radius)
    {
        return;
    }
    const float centerX{left + 0.5F};
    const float centerY{top + 0.5F};
    const float penX{posX < centerX ? posX + radius - left : left + 1.0F - (posX - radius)};
    const float penY{posY < centerY ? posY + radius - top : top + 1.0F - (posY - radius)};
    const float offset{random(2L) == 0L ? jitter : -jitter};
    if (penX <= penY)
    {
        velX = -velX;
        velY += offset;
        posX += posX < centerX ? -penX : penX;
    }
    else
    {
        velY = -velY;
        velX += offset;
        posY += posY < centerY ? -penY : penY;
    }
}

/**
 * @brief Advances the ball one physics tick in sub-steps, ending the level when it reaches the bottom row.
 */
void BouncingBallMode::tick()
{
    const float g{millis() - levelMillis > gravityBoostAfter ? gravityBoosted : gravity};
    const float maxX{static_cast<float>(GRID_COLUMNS) - 1.0F};
    for (uint8_t substep{0U}; substep < substeps; ++substep)
    {
        velY += g;
        posX += velX / static_cast<float>(substeps);
        posY += velY / static_cast<float>(substeps);
        if (posX < 0.0F)
        {
            posX = 0.0F;
            velX = -velX;
        }
        if (posX > maxX)
        {
            posX = maxX;
            velX = -velX;
        }
        if (posY < 0.0F)
        {
            posY = 0.0F;
            velY = -velY;
        }
        for (size_t i{0U}; i < obstacleCount; ++i)
        {
            for (uint8_t c{0U}; c < obstacles[i].length; ++c)
            {
                collide(static_cast<uint8_t>(obstacles[i].x + c), obstacles[i].y);
            }
        }
        if (posY >= static_cast<float>(GRID_ROWS) - 1.0F)
        {
            stage = Stage::END;
            return;
        }
    }
}

/**
 * @brief Draws the obstacles, and the ball while the level runs.
 */
void BouncingBallMode::render() const
{
    Display.fillFrame(0U);
    for (size_t i{0U}; i < obstacleCount; ++i)
    {
        for (uint8_t c{0U}; c < obstacles[i].length; ++c)
        {
            Display.setPixel(static_cast<uint8_t>(obstacles[i].x + c), obstacles[i].y, obstacleBrightness);
        }
    }
    if (stage == Stage::RUN)
    {
        // Cell n covers [n, n+1), rounding would make a ball resting on a segment flicker between it and the row above.
        const int ballX{static_cast<int>(std::floor(posX))};
        const int ballY{static_cast<int>(std::floor(posY))};
        if (ballX >= 0 && ballX < static_cast<int>(GRID_COLUMNS) && ballY >= 0 && ballY < static_cast<int>(GRID_ROWS))
        {
            Display.setPixel(static_cast<uint8_t>(ballX), static_cast<uint8_t>(ballY), UINT8_MAX);
        }
    }
}

#endif // MODE_BOUNCINGBALL
