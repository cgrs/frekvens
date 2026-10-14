#if MODE_TETRIS

#include "modes/TetrisMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 8U, __STRING(MODE_TETRIS) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 8U, __STRING(MODE_TETRIS) " is not compatible with this device's display size.");

/**
 * @brief Clears the display, empties the field and drops the first piece.
 */
void TetrisMode::begin()
{
    Display.fillFrame(0U);
    resetGame();
    spawnPiece();
    lastMillis = millis();
}

/**
 * @brief Advances the self-playing game and renders the field when a step has elapsed.
 *
 * Moves the active piece toward the position chosen by planAi() at the current gravity interval, blinks
 * completed rows before collapsing them, and flashes the field before restarting on game over.
 */
void TetrisMode::handle()
{
    const unsigned long interval = stage == Stage::RUNNING    ? dropInterval
                                 : stage == Stage::CLEARING ? blinkInterval
                                                            : flashInterval;
    if (millis() - lastMillis < interval)
    {
        return;
    }
    lastMillis = millis();
    switch (stage)
    {
    case Stage::RUNNING:
        tickRunning();
        break;
    case Stage::CLEARING:
        blinkVisible = !blinkVisible;
        ++toggles;
        if (toggles >= sequenceToggles)
        {
            collapse();
            spawnPiece();
        }
        break;
    case Stage::GAMEOVER:
        fieldVisible = !fieldVisible;
        ++toggles;
        if (toggles >= sequenceToggles)
        {
            resetGame();
            spawnPiece();
        }
        break;
    }
    render();
}

/**
 * @brief Checks whether a piece overlaps the walls, the floor or locked cells.
 *
 * @param type Piece type.
 * @param rot Rotation of the piece.
 * @param px Horizontal position of the piece's 4x4 box.
 * @param py Vertical position of the piece's 4x4 box.
 * @param _field Field to test against.
 * @return true if the piece collides.
 */
bool TetrisMode::collides(uint8_t type, uint8_t rot, int8_t px, int8_t py, const Field &_field)
{
    const uint16_t mask{pieces[type][rot]};
    for (uint8_t idx{0U}; idx < 16U; ++idx)
    {
        if ((mask & (1U << idx)) == 0U)
        {
            continue;
        }
        const int cx{px + (idx % 4)};
        const int cy{py + (idx / 4)};
        if (cx < 0 || cx >= static_cast<int>(GRID_COLUMNS) || cy >= static_cast<int>(GRID_ROWS))
        {
            return true;
        }
        if (cy >= 0 && _field[cy][cx] != 0U)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Measures the stack height of a column.
 */
uint8_t TetrisMode::columnHeight(const Field &_field, uint8_t col)
{
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        if (_field[y][col] != 0U)
        {
            return static_cast<uint8_t>(GRID_ROWS - y);
        }
    }
    return 0U;
}

/**
 * @brief Counts empty cells that have a locked cell above them.
 */
uint16_t TetrisMode::countHoles(const Field &_field)
{
    uint16_t holes{0U};
    for (uint8_t col{0U}; col < GRID_COLUMNS; ++col)
    {
        bool filled{false};
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            if (_field[y][col] != 0U)
            {
                filled = true;
            }
            else if (filled)
            {
                ++holes;
            }
        }
    }
    return holes;
}

/**
 * @brief Counts completed rows.
 */
uint16_t TetrisMode::countFullRows(const Field &_field)
{
    uint16_t rows{0U};
    for (const auto &row : _field)
    {
        bool full{true};
        for (const uint8_t cell : row)
        {
            if (cell == 0U)
            {
                full = false;
                break;
            }
        }
        if (full)
        {
            ++rows;
        }
    }
    return rows;
}

/**
 * @brief Scores a field after a placement: rewards cleared rows, penalizes new holes, height and bumpiness.
 *
 * @param _field Field after the placement.
 * @param holesBefore Hole count before the placement.
 */
int32_t TetrisMode::scorePlacement(const Field &_field, uint16_t holesBefore)
{
    const int32_t lines{countFullRows(_field)};
    const int32_t holes{static_cast<int32_t>(countHoles(_field)) - static_cast<int32_t>(holesBefore)};
    int32_t aggregateHeight{0};
    int32_t bumpiness{0};
    int32_t prevHeight{0};
    for (uint8_t col{0U}; col < GRID_COLUMNS; ++col)
    {
        const int32_t height{columnHeight(_field, col)};
        aggregateHeight += height;
        if (col > 0U)
        {
            bumpiness += height > prevHeight ? height - prevHeight : prevHeight - height;
        }
        prevHeight = height;
    }
    return (76 * lines) - (256 * holes) - (16 * aggregateHeight) - (10 * bumpiness);
}

/**
 * @brief Drops a piece straight down in the current field and writes the result to `out`.
 */
void TetrisMode::simulateDrop(uint8_t type, uint8_t rot, uint8_t left, Field &out) const
{
    int8_t py{0};
    while (!collides(type, rot, static_cast<int8_t>(left), static_cast<int8_t>(py + 1), field))
    {
        ++py;
    }
    out = field;
    const uint16_t mask{pieces[type][rot]};
    for (uint8_t idx{0U}; idx < 16U; ++idx)
    {
        if ((mask & (1U << idx)) != 0U)
        {
            out[py + (idx / 4U)][left + (idx % 4U)] = brightness[type];
        }
    }
}

/**
 * @brief Picks the rotation and column with the best heuristic score for the active piece.
 */
void TetrisMode::planAi()
{
    Field sim{};
    const uint16_t holesBefore{countHoles(field)};
    bool found{false};
    int32_t bestScore{0};
    targetRot = pieceRot;
    targetX = pieceX;
    for (uint8_t rot{0U}; rot < 4U; ++rot)
    {
        for (uint8_t col{0U}; col < GRID_COLUMNS; ++col)
        {
            if (collides(pieceType, rot, static_cast<int8_t>(col), 0, field))
            {
                continue;
            }
            simulateDrop(pieceType, rot, col, sim);
            const int32_t score{scorePlacement(sim, holesBefore)};
            if (!found || score > bestScore)
            {
                found = true;
                bestScore = score;
                targetRot = rot;
                targetX = static_cast<int8_t>(col);
            }
        }
    }
}

/**
 * @brief Rotates, then shifts, then lowers the active piece one step toward its target.
 */
void TetrisMode::tickRunning()
{
    if (pieceRot < targetRot && !collides(pieceType, static_cast<uint8_t>(pieceRot + 1U), pieceX, pieceY, field))
    {
        ++pieceRot;
        return;
    }
    if (pieceX != targetX)
    {
        const int8_t dir{static_cast<int8_t>(targetX > pieceX ? 1 : -1)};
        if (!collides(pieceType, pieceRot, static_cast<int8_t>(pieceX + dir), pieceY, field))
        {
            pieceX = static_cast<int8_t>(pieceX + dir);
            return;
        }
    }
    dropStep();
}

/**
 * @brief Lowers the active piece by one row, or locks it when blocked.
 */
void TetrisMode::dropStep()
{
    if (!collides(pieceType, pieceRot, pieceX, static_cast<int8_t>(pieceY + 1), field))
    {
        ++pieceY;
        return;
    }
    lockPiece();
}

/**
 * @brief Writes the active piece into the field and checks for completed rows.
 */
void TetrisMode::lockPiece()
{
    const uint16_t mask{pieces[pieceType][pieceRot]};
    for (uint8_t idx{0U}; idx < 16U; ++idx)
    {
        if ((mask & (1U << idx)) != 0U)
        {
            field[pieceY + (idx / 4U)][pieceX + (idx % 4U)] = brightness[pieceType];
        }
    }
    startClearing();
}

/**
 * @brief Starts the row blink sequence, or spawns the next piece when no row is complete.
 */
void TetrisMode::startClearing()
{
    clearRowCount = 0U;
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        bool full{true};
        for (const uint8_t cell : field[y])
        {
            if (cell == 0U)
            {
                full = false;
                break;
            }
        }
        if (full && clearRowCount < clearRows.size())
        {
            clearRows[clearRowCount++] = y;
        }
    }
    if (clearRowCount == 0U)
    {
        spawnPiece();
        return;
    }
    toggles = 0U;
    blinkVisible = true;
    stage = Stage::CLEARING;
}

/**
 * @brief Checks whether a row is part of the current clear.
 */
bool TetrisMode::isClearingRow(uint8_t y) const
{
    for (uint8_t idx{0U}; idx < clearRowCount; ++idx)
    {
        if (clearRows[idx] == y)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Removes completed rows, shifts the rest down and updates the gravity.
 */
void TetrisMode::collapse()
{
    int writeY{static_cast<int>(GRID_ROWS) - 1};
    for (int y{static_cast<int>(GRID_ROWS) - 1}; y >= 0; --y)
    {
        if (isClearingRow(static_cast<uint8_t>(y)))
        {
            continue;
        }
        if (writeY != y)
        {
            field[writeY] = field[y];
        }
        --writeY;
    }
    for (; writeY >= 0; --writeY)
    {
        field[writeY].fill(0U);
    }
    linesTotal = static_cast<uint16_t>(linesTotal + clearRowCount);
    clearRowCount = 0U;
    updateSpeed();
}

/**
 * @brief Shortens the gravity interval as more lines are cleared.
 */
void TetrisMode::updateSpeed()
{
    const int32_t interval{static_cast<int32_t>(gravityBase) - (static_cast<int32_t>(gravityStep) * linesTotal)};
    dropInterval = interval < static_cast<int32_t>(gravityMin) ? gravityMin : static_cast<uint16_t>(interval);
}

/**
 * @brief Spawns a random piece, plans its placement, and ends the game if it has no room.
 */
void TetrisMode::spawnPiece()
{
    pieceType = static_cast<uint8_t>(random(pieceCount));
    pieceRot = 0U;
    pieceX = spawnX;
    pieceY = 0;
    planAi();
    if (collides(pieceType, pieceRot, pieceX, pieceY, field))
    {
        startGameOver();
        return;
    }
    stage = Stage::RUNNING;
}

/**
 * @brief Starts the game over flash sequence.
 */
void TetrisMode::startGameOver()
{
    toggles = 0U;
    fieldVisible = true;
    stage = Stage::GAMEOVER;
}

/**
 * @brief Empties the field and resets lines and gravity.
 */
void TetrisMode::resetGame()
{
    for (auto &row : field)
    {
        row.fill(0U);
    }
    linesTotal = 0U;
    clearRowCount = 0U;
    updateSpeed();
}

/**
 * @brief Draws the locked cells and, while running, the active piece.
 */
void TetrisMode::render() const
{
    Display.fillFrame(0U);
    if (stage == Stage::GAMEOVER && !fieldVisible)
    {
        return;
    }
    for (uint8_t y{0U}; y < GRID_ROWS; ++y)
    {
        if (stage == Stage::CLEARING && !blinkVisible && isClearingRow(y))
        {
            continue;
        }
        for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
        {
            if (field[y][x] != 0U)
            {
                Display.setPixel(x, y, field[y][x]);
            }
        }
    }
    if (stage == Stage::RUNNING)
    {
        const uint16_t mask{pieces[pieceType][pieceRot]};
        for (uint8_t idx{0U}; idx < 16U; ++idx)
        {
            if ((mask & (1U << idx)) == 0U)
            {
                continue;
            }
            const int x{pieceX + (idx % 4)};
            const int y{pieceY + (idx / 4)};
            if (x >= 0 && x < static_cast<int>(GRID_COLUMNS) && y >= 0 && y < static_cast<int>(GRID_ROWS))
            {
                Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), brightness[pieceType]);
            }
        }
    }
}

#endif // MODE_TETRIS
