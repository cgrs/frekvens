#pragma once

#if MODE_FACE

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class FaceMode final : public ModeModule
{
private:
    static constexpr uint8_t brightnessMid{140U};
    static constexpr size_t tearsMax{4U};

    /**
     * @brief Interval timer, ready on first use after being forced.
     */
    struct Timer
    {
        unsigned long lastMillis{0UL};
        bool first{true};

        bool isReady(unsigned long interval)
        {
            if (first || millis() - lastMillis >= interval)
            {
                first = false;
                lastMillis = millis();
                return true;
            }
            return false;
        }

        void reset()
        {
            lastMillis = millis();
            first = false;
        }

        void forceReady()
        {
            first = true;
        }
    };

    enum class Expression : uint8_t
    {
        NEUTRAL,
        LAUGH,
        CRYING,
        SHOCKED,
        EXCITED,
        COUNT,
    };

    enum class IdleMouth : uint8_t
    {
        LINE,
        SMILE,
        OPEN_SMALL,
        COUNT,
    };

    struct Tear
    {
        int8_t x{-1};
        int8_t y{-1};
    };

    Timer gazeTimer{};
    Timer easeTimer{};
    Timer blinkTimer{};
    Timer blinkEndTimer{};
    Timer expressionTimer{};
    Timer expressionHoldTimer{};
    Timer mouthTimer{};
    Timer tearSpawnTimer{};
    Timer tearFallTimer{};

    int8_t gazeX{0};
    int8_t gazeY{0};
    int8_t gazeTargetX{0};
    int8_t gazeTargetY{0};

    unsigned long gazeInterval{0UL};
    unsigned long blinkInterval{0UL};
    unsigned long expressionInterval{0UL};
    unsigned long holdInterval{0UL};
    unsigned long mouthInterval{0UL};

    bool blinking{false};
    bool gazeMoving{false};
    bool inExpression{false};
    bool tearLeftNext{true};

    Expression expression{Expression::NEUTRAL};
    Expression expressionLast{Expression::NEUTRAL};

    IdleMouth idleMouth{IdleMouth::LINE};
    IdleMouth idleMouthLast{IdleMouth::LINE};

    std::array<Tear, tearsMax> tears{};

    static void pixel(int x, int y, uint8_t brightness);
    static void block(int x, int y);
    static void outline(int x, int y);
    static void plus(int x, int y);

    void clearTears();
    void updateGaze(bool allow);
    void updateBlink(bool allow);
    void updateExpression();
    void updateIdleMouth(bool allow);
    void updateTears(bool crying);
    void drawEyes() const;
    void drawMouth() const;
    void drawTears() const;
    void render() const;

public:
    static constexpr std::string_view name{"Face"};

    explicit FaceMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_FACE
