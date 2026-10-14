#if MODE_FACE

#include "modes/FaceMode.h"

#include "services/DisplayService.h"

static_assert(GRID_COLUMNS >= 16U, __STRING(MODE_FACE) " is not compatible with this device's display size.");
static_assert(GRID_ROWS >= 16U, __STRING(MODE_FACE) " is not compatible with this device's display size.");

/**
 * @brief Resets gaze, blink, expression and mouth state, and schedules their next changes.
 */
void FaceMode::begin()
{
    Display.fillFrame(0U);
    gazeX = 0;
    gazeY = 0;
    gazeTargetX = static_cast<int8_t>(random(-2L, 3L));
    gazeTargetY = static_cast<int8_t>(random(-1L, 2L));
    gazeMoving = true;
    blinking = false;
    inExpression = false;
    expression = Expression::NEUTRAL;
    expressionLast = Expression::NEUTRAL;
    idleMouth = IdleMouth::LINE;
    idleMouthLast = IdleMouth::LINE;
    clearTears();
    gazeInterval = static_cast<unsigned long>(random(2800L, 4801L));
    blinkInterval = static_cast<unsigned long>(random(2800L, 5501L));
    expressionInterval = static_cast<unsigned long>(random(30000L, 60001L));
    holdInterval = static_cast<unsigned long>(random(1600L, 3001L));
    mouthInterval = static_cast<unsigned long>(random(14000L, 16001L));
    gazeTimer.reset();
    easeTimer.reset();
    blinkTimer.reset();
    expressionTimer.reset();
    mouthTimer.reset();
}

/**
 * @brief Updates the face state and redraws it.
 */
void FaceMode::handle()
{
    updateExpression();
    const bool neutral{!inExpression};
    updateGaze(neutral);
    updateBlink(neutral);
    updateIdleMouth(neutral);
    updateTears(inExpression && expression == Expression::CRYING);
    render();
}

/**
 * @brief Sets a pixel at full brightness unless the coordinates fall outside the display.
 */
void FaceMode::pixel(int x, int y, uint8_t brightness)
{
    if (x < 0 || x >= static_cast<int>(GRID_COLUMNS) || y < 0 || y >= static_cast<int>(GRID_ROWS))
    {
        return;
    }
    Display.setPixel(static_cast<uint8_t>(x), static_cast<uint8_t>(y), brightness);
}

/**
 * @brief Draws a solid 3x3 block.
 */
void FaceMode::block(int x, int y)
{
    for (int dy{0}; dy < 3; ++dy)
    {
        for (int dx{0}; dx < 3; ++dx)
        {
            pixel(x + dx, y + dy, UINT8_MAX);
        }
    }
}

/**
 * @brief Draws a 4x4 square outline.
 */
void FaceMode::outline(int x, int y)
{
    for (int i{0}; i < 4; ++i)
    {
        pixel(x + i, y, UINT8_MAX);
        pixel(x + i, y + 3, UINT8_MAX);
        pixel(x, y + i, UINT8_MAX);
        pixel(x + 3, y + i, UINT8_MAX);
    }
}

/**
 * @brief Draws a plus sign centered on the given pixel.
 */
void FaceMode::plus(int x, int y)
{
    pixel(x, y, UINT8_MAX);
    pixel(x - 1, y, UINT8_MAX);
    pixel(x + 1, y, UINT8_MAX);
    pixel(x, y - 1, UINT8_MAX);
    pixel(x, y + 1, UINT8_MAX);
}

/**
 * @brief Removes all tears.
 */
void FaceMode::clearTears()
{
    tears.fill(Tear{});
}

/**
 * @brief Picks a new gaze target now and then, and eases the eyes toward it.
 */
void FaceMode::updateGaze(bool allow)
{
    if (!allow)
    {
        return;
    }
    if (gazeTimer.isReady(gazeInterval))
    {
        gazeTargetX = static_cast<int8_t>(random(-2L, 3L));
        gazeTargetY = static_cast<int8_t>(random(-1L, 2L));
        gazeInterval = static_cast<unsigned long>(random(2800L, 4801L));
        gazeMoving = true;
    }
    if (easeTimer.isReady(90UL))
    {
        gazeX = static_cast<int8_t>(gazeX + (gazeX < gazeTargetX ? 1 : (gazeX > gazeTargetX ? -1 : 0)));
        gazeY = static_cast<int8_t>(gazeY + (gazeY < gazeTargetY ? 1 : (gazeY > gazeTargetY ? -1 : 0)));
        if (gazeMoving && gazeX == gazeTargetX && gazeY == gazeTargetY)
        {
            gazeTimer.reset();
            gazeMoving = false;
        }
    }
}

/**
 * @brief Blinks at random intervals for 130 ms.
 */
void FaceMode::updateBlink(bool allow)
{
    if (!allow)
    {
        blinking = false;
        return;
    }
    if (!blinking && blinkTimer.isReady(blinkInterval))
    {
        blinking = true;
        blinkEndTimer.reset();
        blinkInterval = static_cast<unsigned long>(random(2800L, 5501L));
    }
    if (blinking && blinkEndTimer.isReady(130UL))
    {
        blinking = false;
    }
}

/**
 * @brief Starts a random expression every 30 to 60 seconds, after the gaze has recentered, and ends it after its hold time.
 */
void FaceMode::updateExpression()
{
    if (inExpression)
    {
        if (expressionHoldTimer.isReady(holdInterval))
        {
            inExpression = false;
            expression = Expression::NEUTRAL;
            clearTears();
            idleMouth = IdleMouth::LINE;
            idleMouthLast = IdleMouth::LINE;
            mouthTimer.reset();
            mouthInterval = static_cast<unsigned long>(random(14000L, 16001L));
            expressionTimer.reset();
            expressionInterval = static_cast<unsigned long>(random(30000L, 60001L));
        }
        return;
    }
    if (!expressionTimer.isReady(expressionInterval))
    {
        return;
    }
    if (gazeX != 0 || gazeY != 0)
    {
        gazeTargetX = 0;
        gazeTargetY = 0;
        expressionTimer.forceReady();
        return;
    }
    Expression picked{};
    do
    {
        picked = static_cast<Expression>(random(1L, static_cast<long>(Expression::COUNT)));
    } while (picked == expressionLast);
    expressionLast = picked;
    expression = picked;
    inExpression = true;
    holdInterval = static_cast<unsigned long>(random(1600L, 3001L));
    expressionHoldTimer.reset();
}

/**
 * @brief Switches to a different idle mouth every 14 to 16 seconds.
 */
void FaceMode::updateIdleMouth(bool allow)
{
    if (!allow)
    {
        return;
    }
    if (mouthTimer.isReady(mouthInterval))
    {
        IdleMouth picked{};
        do
        {
            picked = static_cast<IdleMouth>(random(0L, static_cast<long>(IdleMouth::COUNT)));
        } while (picked == idleMouthLast);
        idleMouthLast = picked;
        idleMouth = picked;
        mouthInterval = static_cast<unsigned long>(random(14000L, 16001L));
    }
}

/**
 * @brief Spawns tears while crying and drops them one row at a time.
 */
void FaceMode::updateTears(bool crying)
{
    if (crying && tearSpawnTimer.isReady(350UL))
    {
        for (Tear &tear : tears)
        {
            if (tear.x < 0)
            {
                tear.x = static_cast<int8_t>(tearLeftNext ? 2 : GRID_COLUMNS - 3U);
                tear.y = 7;
                tearLeftNext = !tearLeftNext;
                break;
            }
        }
    }
    if (tearFallTimer.isReady(120UL))
    {
        for (Tear &tear : tears)
        {
            if (tear.x >= 0)
            {
                ++tear.y;
                if (tear.y >= static_cast<int8_t>(GRID_ROWS))
                {
                    tear = Tear{};
                }
            }
        }
    }
}

/**
 * @brief Draws the eyes for the current expression, or the idle eyes, shifted by the gaze.
 */
void FaceMode::drawEyes() const
{
    if (!inExpression)
    {
        if (blinking)
        {
            for (int dx{0}; dx < 3; ++dx)
            {
                pixel(3 + gazeX + dx, 6 + gazeY, UINT8_MAX);
                pixel(10 + gazeX + dx, 6 + gazeY, UINT8_MAX);
            }
        }
        else
        {
            block(3 + gazeX, 5 + gazeY);
            block(10 + gazeX, 5 + gazeY);
        }
        return;
    }
    switch (expression)
    {
    case Expression::LAUGH:
    {
        const int left{3 + gazeX};
        const int right{10 + gazeX};
        const int top{5 + gazeY};
        pixel(left, top + 1, UINT8_MAX);
        pixel(left + 1, top, UINT8_MAX);
        pixel(left + 2, top + 1, UINT8_MAX);
        pixel(right, top + 1, UINT8_MAX);
        pixel(right + 1, top, UINT8_MAX);
        pixel(right + 2, top + 1, UINT8_MAX);
        break;
    }
    case Expression::CRYING:
        block(3 + gazeX, 5 + gazeY);
        block(10 + gazeX, 5 + gazeY);
        break;
    case Expression::SHOCKED:
        outline(2, 4);
        outline(10, 4);
        break;
    case Expression::EXCITED:
        plus(4 + gazeX, 6 + gazeY);
        plus(11 + gazeX, 6 + gazeY);
        break;
    default:
        break;
    }
}

/**
 * @brief Draws the mouth for the current expression, or the idle mouth.
 */
void FaceMode::drawMouth() const
{
    if (!inExpression)
    {
        switch (idleMouth)
        {
        case IdleMouth::SMILE:
            for (int x{5}; x <= 10; ++x)
            {
                pixel(x, 12, UINT8_MAX);
            }
            pixel(4, 11, UINT8_MAX);
            pixel(11, 11, UINT8_MAX);
            break;
        case IdleMouth::OPEN_SMALL:
            for (int x{6}; x <= 9; ++x)
            {
                pixel(x, 12, UINT8_MAX);
            }
            pixel(7, 13, UINT8_MAX);
            pixel(8, 13, UINT8_MAX);
            break;
        case IdleMouth::LINE:
        default:
            for (int x{5}; x <= 10; ++x)
            {
                pixel(x, 12, UINT8_MAX);
            }
            break;
        }
        return;
    }
    switch (expression)
    {
    case Expression::LAUGH:
        for (int x{6}; x <= 9; ++x)
        {
            pixel(x, 11, UINT8_MAX);
        }
        pixel(5, 12, UINT8_MAX);
        pixel(10, 12, UINT8_MAX);
        for (int x{5}; x <= 10; ++x)
        {
            pixel(x, 13, UINT8_MAX);
        }
        break;
    case Expression::CRYING:
        for (int x{5}; x <= 10; ++x)
        {
            pixel(x, 12, UINT8_MAX);
        }
        pixel(4, 13, UINT8_MAX);
        pixel(11, 13, UINT8_MAX);
        break;
    case Expression::SHOCKED:
        outline(6, 11);
        break;
    case Expression::EXCITED:
        pixel(4, 11, UINT8_MAX);
        pixel(11, 11, UINT8_MAX);
        for (int x{5}; x <= 10; ++x)
        {
            pixel(x, 12, UINT8_MAX);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Draws the falling tears at medium brightness.
 */
void FaceMode::drawTears() const
{
    for (const Tear &tear : tears)
    {
        if (tear.x >= 0)
        {
            pixel(tear.x, tear.y, brightnessMid);
        }
    }
}

/**
 * @brief Redraws the whole face.
 */
void FaceMode::render() const
{
    Display.fillFrame(0U);
    drawEyes();
    drawMouth();
    drawTears();
}

#endif // MODE_FACE
