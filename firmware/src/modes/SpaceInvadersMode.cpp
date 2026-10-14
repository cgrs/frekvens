#if MODE_SPACEINVADERS

#include "modes/SpaceInvadersMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_SPACEINVADERS) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 16U, __STRING(MODE_SPACEINVADERS) " is not compatible with this device's display size.");

/**
 * @brief Clears the display and starts a new game.
 */
void SpaceInvadersMode::begin()
{
    Display.fillFrame(0U);
    resetGame();
    stage = Stage::RUNNING;
    lastMarch = lastMove = lastBullet = millis();
}

/**
 * @brief Advances the self-playing game and renders the scene.
 *
 * Marches the formation, moves the ship under the nearest column and fires, at their own intervals,
 * then blinks on game over and pauses between waves.
 */
void SpaceInvadersMode::handle()
{
    switch (stage)
    {
    case Stage::RUNNING:
    {
        const unsigned long marchEvery{static_cast<unsigned long>(waveIntervalMs * random(90, 111) / 100)};
        if (millis() - lastMarch >= marchEvery)
        {
            lastMarch = millis();
            updateFormation();
            if (stage != Stage::RUNNING)
            {
                break;
            }
        }
        if (millis() - lastMove >= moveInterval)
        {
            lastMove = millis();
            updateShip();
        }
        if (millis() - lastBullet >= bulletInterval)
        {
            lastBullet = millis();
            updateBullet();
        }
        break;
    }
    case Stage::GAME_OVER_BLINK:
        if (millis() - lastSequence < blinkInterval)
        {
            return;
        }
        lastSequence = millis();
        blinkVisible = !blinkVisible;
        ++blinkToggles;
        if (blinkToggles >= blinkTogglesMax)
        {
            Display.fillFrame(0U);
            resetGame();
            stage = Stage::RUNNING;
        }
        break;
    case Stage::WAVE_DELAY:
        if (millis() - lastSequence < wavePause)
        {
            return;
        }
        spawnWave();
        stage = Stage::RUNNING;
        break;
    }
    render();
}

/**
 * @brief Draws a pixel, ignoring coordinates outside the display.
 */
void SpaceInvadersMode::plot(int x, int y, uint8_t brightness)
{
    if (x >= 0 && x < static_cast<int>(GRID_COLUMNS) && y >= 0 && y < static_cast<int>(GRID_ROWS))
    {
        Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), brightness);
    }
}

/**
 * @brief Current column of an enemy column, including the formation offset.
 */
int SpaceInvadersMode::enemyX(uint8_t col) const { return colStartX + (col * colSpacing) + colOffset; }

/**
 * @brief Current row of an enemy row, including the formation offset.
 */
int SpaceInvadersMode::enemyY(uint8_t row) const { return baseRowY + (row * rowSpacing) + rowOffset; }

/**
 * @brief Resets the ship and spawns the first wave.
 */
void SpaceInvadersMode::resetGame()
{
    shipX = shipStartX;
    targetX = shipStartX;
    blinkVisible = true;
    blinkToggles = 0U;
    spawnWave();
}

/**
 * @brief Spawns a full formation with a random direction and march speed.
 */
void SpaceInvadersMode::spawnWave()
{
    for (auto &row : alive)
    {
        row.fill(true);
    }
    colOffset = 0;
    rowOffset = 0;
    marchDir = random(2) != 0 ? 1 : -1;
    waveIntervalMs = static_cast<uint16_t>(marchInterval * random(85, 116) / 100);
    enemiesAlive = enemyRows * enemyCols;
    bulletActive = false;
}

/**
 * @brief Marches the formation sideways, or one row down and reversed at an edge; ends the game when it lands.
 */
void SpaceInvadersMode::updateFormation()
{
    bool atEdge{false};
    for (uint8_t row{0U}; row < enemyRows && !atEdge; ++row)
    {
        for (uint8_t col{0U}; col < enemyCols; ++col)
        {
            if (!alive[row][col])
            {
                continue;
            }
            const int ex{enemyX(col)};
            if (ex <= 0 || ex >= static_cast<int>(GRID_COLUMNS - 1U))
            {
                atEdge = true;
                break;
            }
        }
    }
    if (atEdge)
    {
        ++rowOffset;
        marchDir = static_cast<int8_t>(-marchDir);
    }
    else
    {
        colOffset = static_cast<int8_t>(colOffset + marchDir);
    }

    for (uint8_t row{0U}; row < enemyRows; ++row)
    {
        for (uint8_t col{0U}; col < enemyCols; ++col)
        {
            if (alive[row][col] && enemyY(row) >= static_cast<int>(gameOverY))
            {
                startGameOver();
                return;
            }
        }
    }
}

/**
 * @brief Moves the ship one column toward the nearest column of enemies and fires when aligned.
 */
void SpaceInvadersMode::updateShip()
{
    int bestDist{INT8_MAX};
    int bestX{shipX};
    for (uint8_t col{0U}; col < enemyCols; ++col)
    {
        for (int row{enemyRows - 1}; row >= 0; --row)
        {
            if (!alive[row][col])
            {
                continue;
            }
            const int ex{enemyX(col)};
            const int dist{ex > shipX ? ex - shipX : shipX - ex};
            if (dist < bestDist)
            {
                bestDist = dist;
                bestX = ex;
            }
            break;
        }
    }
    targetX = static_cast<int8_t>(bestX);

    if (shipX < targetX)
    {
        ++shipX;
    }
    else if (shipX > targetX)
    {
        --shipX;
    }

    bool aligned{false};
    for (uint8_t col{0U}; col < enemyCols && !aligned; ++col)
    {
        for (int row{enemyRows - 1}; row >= 0; --row)
        {
            if (!alive[row][col])
            {
                continue;
            }
            aligned = enemyX(col) == shipX;
            break;
        }
    }

    if (!bulletActive && aligned)
    {
        bulletX = shipX;
        bulletY = bulletSpawnY;
        bulletActive = true;
        lastBullet = millis();
    }
}

/**
 * @brief Moves the bullet up one row and removes the enemy it hits.
 */
void SpaceInvadersMode::updateBullet()
{
    if (!bulletActive)
    {
        return;
    }
    --bulletY;
    if (bulletY < 0)
    {
        bulletActive = false;
        return;
    }
    for (uint8_t row{0U}; row < enemyRows; ++row)
    {
        for (uint8_t col{0U}; col < enemyCols; ++col)
        {
            if (alive[row][col] && enemyX(col) == bulletX && enemyY(row) == bulletY)
            {
                alive[row][col] = false;
                --enemiesAlive;
                bulletActive = false;
                if (enemiesAlive == 0U)
                {
                    startWaveClear();
                }
                return;
            }
        }
    }
}

/**
 * @brief Pauses before the next wave.
 */
void SpaceInvadersMode::startWaveClear()
{
    blinkVisible = true;
    blinkToggles = 0U;
    stage = Stage::WAVE_DELAY;
    lastSequence = millis();
}

/**
 * @brief Starts the game over blink.
 */
void SpaceInvadersMode::startGameOver()
{
    blinkVisible = true;
    blinkToggles = 0U;
    stage = Stage::GAME_OVER_BLINK;
    lastSequence = millis();
}

/**
 * @brief Draws the enemies, the ship and the bullet, hiding them during the off phase of the game over blink.
 */
void SpaceInvadersMode::render() const
{
    Display.fillFrame(0U);

    if (stage == Stage::GAME_OVER_BLINK && !blinkVisible)
    {
        return;
    }

    for (uint8_t row{0U}; row < enemyRows; ++row)
    {
        for (uint8_t col{0U}; col < enemyCols; ++col)
        {
            if (alive[row][col])
            {
                plot(enemyX(col), enemyY(row), brightnessMid);
            }
        }
    }

    for (int dx{-1}; dx <= 1; ++dx)
    {
        plot(shipX + dx, shipY, brightnessFull);
    }
    plot(shipX, shipY - 1, brightnessFull);

    if (bulletActive)
    {
        plot(bulletX, bulletY, brightnessHigh);
    }
}

#endif // MODE_SPACEINVADERS
