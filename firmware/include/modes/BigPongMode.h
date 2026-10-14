#pragma once

#if MODE_BIGPONG

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

class BigPongMode final : public ModeModule
{
private:
    static constexpr uint8_t paddleWidth{4U};
    static constexpr uint8_t paddleTopY{0U};
    static constexpr uint8_t paddleBottomY{GRID_ROWS - 1U};
    static constexpr uint8_t midLineY{(GRID_ROWS - 1U) / 2U};
    static constexpr uint8_t brightnessFull{255U};
    static constexpr uint8_t brightnessFaint{10U};
    static constexpr float baseSpeed{1.05F};
    static constexpr float maxSpeedFactor{1.5F};
    static constexpr float speedUp{1.05F};
    static constexpr float nudge{0.2F};
    static constexpr uint8_t moveInterval{75U};
    static constexpr uint8_t goalBlinkInterval{150U};
    static constexpr uint16_t serveDelay{500U};
    static constexpr uint8_t goalBlinkToggles{8U};
    static constexpr uint8_t serveAngleMaxDeg{25U};
    static constexpr uint8_t aimErrorMax{1U};
    static constexpr uint16_t predictMaxSteps{512U};

    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        RUNNING,
        GOAL_BLINK,
        SERVE_DELAY,
    };

    enum class Player : uint8_t // NOLINT(performance-enum-size)
    {
        TOP,
        BOTTOM,
    };

    Stage stage{Stage::RUNNING};

    float bx{(GRID_COLUMNS - 1U) / 2.0F};
    float by{(GRID_ROWS - 1U) / 2.0F};
    float vx{0.0F};
    float vy{baseSpeed};

    int8_t topPaddleX{(GRID_COLUMNS - paddleWidth) / 2};
    int8_t bottomPaddleX{(GRID_COLUMNS - paddleWidth) / 2};
    int8_t topTargetX{(GRID_COLUMNS - paddleWidth) / 2};
    int8_t bottomTargetX{(GRID_COLUMNS - paddleWidth) / 2};
    int8_t aimErrorTop{0};
    int8_t aimErrorBottom{0};

    Player scoringPlayer{Player::TOP};
    float serveDirY{1.0F};

    bool blinkVisible{true};
    uint8_t blinkToggles{0U};

    unsigned long lastMillis{0UL};

    static int8_t clampPaddle(int paddle);
    static int8_t rollAimError();

    void tickGame();
    void updateAi();
    [[nodiscard]] int8_t predictLanding(bool top) const;
    void bounceOffPaddle(bool top);
    void scaleBallSpeed();
    void serve(float dirY);
    void startGoal(Player scorer);
    static void drawPaddleRow(int8_t x, uint8_t y);
    void render() const;

public:
    static constexpr std::string_view name{"Big pong"};

    explicit BigPongMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_BIGPONG
