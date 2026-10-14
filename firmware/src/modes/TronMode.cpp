#if MODE_TRON

#include "modes/TronMode.h"

#include "services/DisplayService.h"

#include <algorithm>

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_TRON) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_TRON) " is not compatible with this device's display size.");
static_assert(GRID_COLUMNS <= INT8_MAX && GRID_ROWS <= INT8_MAX,
              __STRING(MODE_TRON) " is not compatible with this device's display size.");

/**
 * @brief Clears the display and starts a new round.
 */
void TronMode::begin()
{
    Display.fillFrame(0U);
    resetBoard();
}

/**
 * @brief Advances the two-rider game, the loser flash and the trail rewind at their own intervals.
 */
void TronMode::handle()
{
    const unsigned long interval = stage == Stage::RUNNING ? stepInterval
                                 : stage == Stage::FLASH ? flashInterval
                                                         : rewindInterval;
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    switch (stage)
    {
    case Stage::RUNNING:
        tickRiders();
        break;
    case Stage::FLASH:
        blinkVisible = !blinkVisible;
        ++blinkToggles;
        if (blinkToggles >= flashToggles)
        {
            stage = Stage::REWIND;
        }
        break;
    case Stage::REWIND:
        rewindTick();
        break;
    }
    render();
}

/**
 * @brief Checks whether a cell is on the display and not covered by a trail.
 */
bool TronMode::cellFree(int8_t x, int8_t y) const
{
    if (x < 0 || x >= static_cast<int8_t>(GRID_COLUMNS) || y < 0 || y >= static_cast<int8_t>(GRID_ROWS))
    {
        return false;
    }
    return trail[y][x] == 0U;
}

/**
 * @brief Like cellFree(), but also treats two extra cells as blocked.
 */
bool TronMode::cellFreeForSearch(int8_t x, int8_t y, int8_t excludeAX, int8_t excludeAY, int8_t excludeBX,
                                 int8_t excludeBY) const
{
    if (!cellFree(x, y))
    {
        return false;
    }
    if (x == excludeAX && y == excludeAY)
    {
        return false;
    }
    return !(x == excludeBX && y == excludeBY);
}

/**
 * @brief Counts the free cells reachable from a start cell.
 *
 * @return Number of reachable cells, or 0 if the start cell is blocked.
 */
uint16_t TronMode::floodFillCount(int8_t startX, int8_t startY, int8_t excludeAX, int8_t excludeAY, int8_t excludeBX,
                                  int8_t excludeBY) const
{
    static constexpr std::array<int8_t, 4U> neighborDx{-1, 1, 0, 0};
    static constexpr std::array<int8_t, 4U> neighborDy{0, 0, -1, 1};
    std::array<std::array<bool, GRID_COLUMNS>, GRID_ROWS> visited{};
    std::array<int8_t, trailMax> queueX{};
    std::array<int8_t, trailMax> queueY{};
    uint16_t head{0U};
    uint16_t tail{0U};
    uint16_t count{0U};
    if (!cellFreeForSearch(startX, startY, excludeAX, excludeAY, excludeBX, excludeBY))
    {
        return 0U;
    }
    queueX[tail] = startX;
    queueY[tail] = startY;
    ++tail;
    visited[startY][startX] = true;
    while (head < tail && count < trailMax)
    {
        const int8_t cx{queueX[head]};
        const int8_t cy{queueY[head]};
        ++head;
        ++count;
        for (uint8_t idx{0U}; idx < 4U; ++idx)
        {
            const int8_t nx{static_cast<int8_t>(cx + neighborDx[idx])};
            const int8_t ny{static_cast<int8_t>(cy + neighborDy[idx])};
            if (!cellFreeForSearch(nx, ny, excludeAX, excludeAY, excludeBX, excludeBY) || visited[ny][nx])
            {
                continue;
            }
            visited[ny][nx] = true;
            queueX[tail] = nx;
            queueY[tail] = ny;
            ++tail;
        }
    }
    return count;
}

/**
 * @brief Puts a rider on the board and starts its trail history.
 */
void TronMode::placeRider(uint8_t idx, int8_t x, int8_t y, int8_t dx, int8_t dy)
{
    riders[idx] = {.x = x, .y = y, .dx = dx, .dy = dy};
    trail[y][x] = riderBrightness[idx];
    histX[idx][0] = x;
    histY[idx][0] = y;
    histLen[idx] = 1U;
}

/**
 * @brief Clears all trails and places both riders on opposite edges.
 */
void TronMode::resetBoard()
{
    for (auto &row : trail)
    {
        row.fill(0U);
    }
    histLen.fill(0U);
    loser.fill(false);
    blinkVisible = true;
    blinkToggles = 0U;
    placeRider(0U, 0, static_cast<int8_t>(GRID_ROWS * 5U / 16U), 1, 0);
    placeRider(1U, static_cast<int8_t>(GRID_COLUMNS - 1U), static_cast<int8_t>(GRID_ROWS * 10U / 16U), -1, 0);
    stage = Stage::RUNNING;
    lastMillis = millis();
}

/**
 * @brief Picks the next cell for a rider among straight, left and right.
 *
 * Prefers the option with the most reachable free cells, with a coin flip to keep going straight on ties.
 *
 * @param idx Rider index.
 * @param[out] targetX Chosen column.
 * @param[out] targetY Chosen row.
 * @param otherPendingX Column the other rider has already chosen, or -1.
 * @param otherPendingY Row the other rider has already chosen, or -1.
 * @return false if the rider has no free cell to move to.
 */
bool TronMode::chooseMove(uint8_t idx, int8_t &targetX, int8_t &targetY, int8_t otherPendingX,
                          int8_t otherPendingY) const
{
    const Rider &rider{riders[idx]};
    const Rider &other{riders[idx == 0U ? 1U : 0U]};
    const std::array<std::array<int8_t, 2U>, moveOptionCount> headings{{
        {rider.dx, rider.dy},
        {static_cast<int8_t>(-rider.dy), rider.dx},
        {rider.dy, static_cast<int8_t>(-rider.dx)},
    }};
    std::array<MoveOption, moveOptionCount> options{};
    std::array<uint16_t, moveOptionCount> scores{};
    uint8_t optionCount{0U};
    for (const auto &heading : headings)
    {
        const int8_t tx{static_cast<int8_t>(rider.x + heading[0])};
        const int8_t ty{static_cast<int8_t>(rider.y + heading[1])};
        if (!cellFree(tx, ty) || (tx == other.x && ty == other.y))
        {
            continue;
        }
        options[optionCount] = {.dx = heading[0], .dy = heading[1], .tx = tx, .ty = ty};
        scores[optionCount] = floodFillCount(tx, ty, other.x, other.y, otherPendingX, otherPendingY);
        ++optionCount;
    }
    if (optionCount == 0U)
    {
        return false;
    }
    uint16_t bestScore{scores[0]};
    for (uint8_t opt{1U}; opt < optionCount; ++opt)
    {
        bestScore = std::max(bestScore, scores[opt]);
    }
    int8_t straightIndex{-1};
    for (uint8_t opt{0U}; opt < optionCount; ++opt)
    {
        if (options[opt].dx == rider.dx && options[opt].dy == rider.dy)
        {
            straightIndex = static_cast<int8_t>(opt);
            break;
        }
    }
    uint8_t choice{0U};
    if (bestScore == 0U)
    {
        choice = static_cast<uint8_t>(random(optionCount));
    }
    else if (straightIndex >= 0 && scores[straightIndex] == bestScore && random(100) < 50)
    {
        choice = static_cast<uint8_t>(straightIndex);
    }
    else
    {
        std::array<uint8_t, moveOptionCount> nearIndices{};
        uint8_t nearCount{0U};
        for (uint8_t opt{0U}; opt < optionCount; ++opt)
        {
            if (static_cast<uint32_t>(scores[opt]) * 10U >= static_cast<uint32_t>(bestScore) * 9U)
            {
                nearIndices[nearCount++] = opt;
            }
        }
        choice = nearIndices[random(nearCount)];
    }
    targetX = options[choice].tx;
    targetY = options[choice].ty;
    return true;
}

/**
 * @brief Moves a rider to a cell, leaving a trail.
 */
void TronMode::commitMove(uint8_t idx, int8_t targetX, int8_t targetY)
{
    Rider &rider{riders[idx]};
    rider.dx = static_cast<int8_t>(targetX - rider.x);
    rider.dy = static_cast<int8_t>(targetY - rider.y);
    rider.x = targetX;
    rider.y = targetY;
    trail[targetY][targetX] = riderBrightness[idx];
    const uint16_t len{histLen[idx]};
    histX[idx][len] = targetX;
    histY[idx][len] = targetY;
    histLen[idx] = static_cast<uint16_t>(len + 1U);
}

/**
 * @brief Moves both riders one cell, ending the round on a crash or a head-on collision.
 */
void TronMode::tickRiders()
{
    std::array<bool, riderCount> hasTarget{};
    std::array<int8_t, riderCount> targetX{};
    std::array<int8_t, riderCount> targetY{};
    for (uint8_t idx{0U}; idx < riderCount; ++idx)
    {
        int8_t pendingX{-1};
        int8_t pendingY{-1};
        if (idx > 0U && hasTarget[idx - 1U])
        {
            pendingX = targetX[idx - 1U];
            pendingY = targetY[idx - 1U];
        }
        hasTarget[idx] = chooseMove(idx, targetX[idx], targetY[idx], pendingX, pendingY);
    }
    if (hasTarget[0] && hasTarget[1] && targetX[0] == targetX[1] && targetY[0] == targetY[1])
    {
        hasTarget.fill(false);
    }
    for (uint8_t idx{0U}; idx < riderCount; ++idx)
    {
        if (hasTarget[idx])
        {
            commitMove(idx, targetX[idx], targetY[idx]);
        }
    }
    if (!hasTarget[0] || !hasTarget[1])
    {
        endRound(!hasTarget[0], !hasTarget[1]);
    }
}

/**
 * @brief Marks the losing riders and starts flashing their trails.
 */
void TronMode::endRound(bool loserOne, bool loserTwo)
{
    loser[0] = loserOne;
    loser[1] = loserTwo;
    blinkVisible = true;
    blinkToggles = 0U;
    stage = Stage::FLASH;
}

/**
 * @brief Erases the last cell of each trail, and restarts once both are gone.
 */
void TronMode::rewindTick()
{
    for (uint8_t idx{0U}; idx < riderCount; ++idx)
    {
        if (histLen[idx] > 0U)
        {
            --histLen[idx];
            trail[histY[idx][histLen[idx]]][histX[idx][histLen[idx]]] = 0U;
        }
    }
    if (histLen[0] == 0U && histLen[1] == 0U)
    {
        resetBoard();
    }
}

/**
 * @brief Draws the trails, hiding the losers' trails during the off phase of the flash.
 */
void TronMode::render() const
{
    Display.fillFrame(0U);
    const bool blinkOff{stage == Stage::FLASH && !blinkVisible};
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            const uint8_t value{trail[y][x]};
            if (value == 0U)
            {
                continue;
            }
            if (blinkOff && loser[value == riderBrightness[0] ? 0U : 1U])
            {
                continue;
            }
            Display.setPixel(x, y, value);
        }
    }
}

#endif // MODE_TRON
