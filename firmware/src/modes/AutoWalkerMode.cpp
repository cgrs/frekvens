#if MODE_AUTOWALKER

#include "modes/AutoWalkerMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <utility>

static_assert(GRID_COLUMNS >= 2U, __STRING(MODE_AUTOWALKER) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 2U, __STRING(MODE_AUTOWALKER) " is not compatible with this device's display size.");

/**
 * @brief Clears the display and starts a new walk.
 */
void AutoWalkerMode::begin()
{
    Display.fillFrame(0U);
    restart();
}

/**
 * @brief Advances the walk, holds the finished picture, then fades it out.
 */
void AutoWalkerMode::handle()
{
    switch (stage)
    {
    case Stage::RUN:
        if (millis() - lastMillis < stepInterval)
        {
            return;
        }
        lastMillis = millis();
        step();
        break;
    case Stage::HOLD:
        if (millis() - lastMillis < holdDuration)
        {
            return;
        }
        lastMillis = millis();
        stage = Stage::FADE;
        break;
    case Stage::FADE:
        if (millis() - lastMillis < fadeInterval)
        {
            return;
        }
        lastMillis = millis();
        fadeOut();
        break;
    }
    render();
}

/**
 * @brief Maps a cell's visit count to its brightness.
 */
uint8_t AutoWalkerMode::brightnessFor(uint8_t count)
{
    if (count <= 1U)
    {
        return brightnessVisit1;
    }
    if (count == 2U)
    {
        return brightnessVisit2;
    }
    return brightnessVisitMax;
}

/**
 * @brief Forgets all visits and places the walker at a random cell.
 */
void AutoWalkerMode::restart()
{
    for (std::array<uint8_t, GRID_COLUMNS> &row : counts)
    {
        row.fill(0U);
    }
    visited = 0U;
    x = static_cast<uint8_t>(random(GRID_COLUMNS));
    y = static_cast<uint8_t>(random(GRID_ROWS));
    lastMillis = millis();
    stage = Stage::RUN;
}

/**
 * @brief Moves the walker to the least visited neighbouring cell, occasionally to a random one.
 */
void AutoWalkerMode::step()
{
    std::array<std::pair<uint8_t, uint8_t>, 4U> neighbors{};
    size_t total{0U};
    if (x > 0U)
    {
        neighbors[total++] = {static_cast<uint8_t>(x - 1U), y};
    }
    if (x < GRID_COLUMNS - 1U)
    {
        neighbors[total++] = {static_cast<uint8_t>(x + 1U), y};
    }
    if (y > 0U)
    {
        neighbors[total++] = {x, static_cast<uint8_t>(y - 1U)};
    }
    if (y < GRID_ROWS - 1U)
    {
        neighbors[total++] = {x, static_cast<uint8_t>(y + 1U)};
    }

    size_t choice{0U};
    if (random(wanderOneIn) == 0L)
    {
        choice = static_cast<size_t>(random(static_cast<long>(total)));
    }
    else
    {
        uint8_t minCount{UINT8_MAX};
        for (size_t i{0U}; i < total; ++i)
        {
            minCount = std::min(minCount, counts[neighbors[i].second][neighbors[i].first]);
        }
        std::array<size_t, 4U> candidates{};
        size_t candidateTotal{0U};
        for (size_t i{0U}; i < total; ++i)
        {
            if (counts[neighbors[i].second][neighbors[i].first] == minCount)
            {
                candidates[candidateTotal++] = i;
            }
        }
        choice = candidates[static_cast<size_t>(random(static_cast<long>(candidateTotal)))];
    }

    x = neighbors[choice].first;
    y = neighbors[choice].second;

    uint8_t &count{counts[y][x]};
    if (count == 0U)
    {
        ++visited;
    }
    if (count < UINT8_MAX)
    {
        ++count;
    }

    if (visited >= cells)
    {
        startHold();
    }
}

/**
 * @brief Snapshots the finished picture for the fade and starts holding it.
 */
void AutoWalkerMode::startHold()
{
    for (size_t row{0U}; row < GRID_ROWS; ++row)
    {
        for (size_t column{0U}; column < GRID_COLUMNS; ++column)
        {
            fade[row][column] = brightnessFor(counts[row][column]);
        }
    }
    fade[y][x] = brightnessWalker;
    stage = Stage::HOLD;
    lastMillis = millis();
}

/**
 * @brief Dims every lit cell one step, and starts a new walk once all are dark.
 */
void AutoWalkerMode::fadeOut()
{
    bool remaining{false};
    for (std::array<uint8_t, GRID_COLUMNS> &row : fade)
    {
        for (uint8_t &value : row)
        {
            value = value > fadeStep ? static_cast<uint8_t>(value - fadeStep) : 0U;
            remaining = remaining || value > 0U;
        }
    }
    if (!remaining)
    {
        restart();
    }
}

/**
 * @brief Draws the visited cells and the walker, or the fading picture.
 */
void AutoWalkerMode::render() const
{
    Display.fillFrame(0U);
    if (stage == Stage::FADE)
    {
        for (size_t row{0U}; row < GRID_ROWS; ++row)
        {
            for (size_t column{0U}; column < GRID_COLUMNS; ++column)
            {
                if (fade[row][column] > 0U)
                {
                    Display.setPixel(static_cast<uint8_t>(column), static_cast<uint8_t>(row), fade[row][column]);
                }
            }
        }
        return;
    }
    for (size_t row{0U}; row < GRID_ROWS; ++row)
    {
        for (size_t column{0U}; column < GRID_COLUMNS; ++column)
        {
            if (counts[row][column] > 0U)
            {
                Display.setPixel(static_cast<uint8_t>(column), static_cast<uint8_t>(row), brightnessFor(counts[row][column]));
            }
        }
    }
    Display.setPixel(x, y, brightnessWalker);
}

#endif // MODE_AUTOWALKER
