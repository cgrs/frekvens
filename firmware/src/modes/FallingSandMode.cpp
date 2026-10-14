#if MODE_FALLINGSAND

#include "modes/FallingSandMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 4U, __STRING(MODE_FALLINGSAND) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 2U, __STRING(MODE_FALLINGSAND) " is not compatible with this device's display size.");

/**
 * @brief Clears the sand, resets the spawn column and starts spawning.
 */
void FallingSandMode::begin()
{
    Display.fillFrame(0U);
    for (std::array<uint8_t, GRID_COLUMNS> &row : amount)
    {
        row.fill(0U);
    }
    spawnColumn = spawnColumnStart;
    wanderDelay = static_cast<unsigned long>(random(static_cast<long>(wanderMin), static_cast<long>(wanderMax) + 1L));
    tickMillis = spawnMillis = drainMillis = wanderMillis = millis();
    stage = Stage::SPAWN;
}

/**
 * @brief Spawns and drops sand until the top row fills up, then drains the pile and starts over.
 */
void FallingSandMode::handle()
{
    switch (stage)
    {
    case Stage::SPAWN:
        if (millis() - tickMillis < tickInterval)
        {
            return;
        }
        tickMillis = millis();
        if (millis() - wanderMillis >= wanderDelay)
        {
            wanderMillis = millis();
            wanderDelay =
                static_cast<unsigned long>(random(static_cast<long>(wanderMin), static_cast<long>(wanderMax) + 1L));
            wander();
        }
        if (millis() - spawnMillis >= spawnInterval)
        {
            spawnMillis = millis();
            spawn();
        }
        physics();
        render();
        if (topRowOccupied())
        {
            stage = Stage::DRAIN;
            drainMillis = millis();
        }
        break;
    case Stage::DRAIN:
        if (millis() - drainMillis < drainInterval)
        {
            return;
        }
        drainMillis = millis();
        drain();
        render();
        if (empty())
        {
            spawnColumn = spawnColumnStart;
            stage = Stage::SPAWN;
        }
        break;
    }
}

/**
 * @brief Checks whether any sand rests in the top row.
 */
bool FallingSandMode::topRowOccupied() const
{
    for (const uint8_t cell : amount[0U])
    {
        if (cell > 0U)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks whether the grid holds no sand.
 */
bool FallingSandMode::empty() const
{
    for (const std::array<uint8_t, GRID_COLUMNS> &row : amount)
    {
        for (const uint8_t cell : row)
        {
            if (cell > 0U)
            {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Moves the spawn column up to two columns sideways, within bounds.
 */
void FallingSandMode::wander()
{
    const long next{static_cast<long>(spawnColumn) + random(-2L, 3L)};
    spawnColumn = static_cast<uint8_t>(constrain(next, static_cast<long>(spawnColumnMin), static_cast<long>(spawnColumnMax)));
}

/**
 * @brief Adds a grain of sand at the top of the spawn column.
 */
void FallingSandMode::spawn()
{
    if (amount[0U][spawnColumn] < UINT8_MAX)
    {
        ++amount[0U][spawnColumn];
    }
}

/**
 * @brief Moves each grain down one row, or diagonally when blocked, from the bottom up.
 */
void FallingSandMode::physics()
{
    const bool preferLeft{random(2L) == 0L};
    const std::array<int, 2U> candidates{preferLeft ? -1 : 1, preferLeft ? 1 : -1};
    for (int y{static_cast<int>(GRID_ROWS) - 2}; y >= 0; --y)
    {
        for (size_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            if (amount[y][x] == 0U)
            {
                continue;
            }
            if (amount[y + 1][x] == 0U)
            {
                --amount[y][x];
                ++amount[y + 1][x];
                continue;
            }
            for (const int dx : candidates)
            {
                if ((dx < 0 && x == 0U) || (dx > 0 && x >= GRID_COLUMNS - 1U))
                {
                    continue;
                }
                const size_t target{static_cast<size_t>(static_cast<int>(x) + dx)};
                if (amount[y + 1][target] == 0U)
                {
                    --amount[y][x];
                    ++amount[y + 1][target];
                    break;
                }
            }
        }
    }
}

/**
 * @brief Shifts the whole pile one row down, dropping the bottom row.
 */
void FallingSandMode::drain()
{
    for (size_t y{GRID_ROWS - 1U}; y > 0U; --y)
    {
        amount[y] = amount[y - 1U];
    }
    amount[0U].fill(0U);
}

/**
 * @brief Draws every occupied cell at full brightness.
 */
void FallingSandMode::render() const
{
    Display.fillFrame(0U);
    for (size_t y{0U}; y < GRID_ROWS; ++y)
    {
        for (size_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            if (amount[y][x] > 0U)
            {
                Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), UINT8_MAX);
            }
        }
    }
}

#endif // MODE_FALLINGSAND
