#pragma once

#if MODE_FLAPPYBIRD

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class FlappyBirdMode final : public ModeModule
{
private:
    static constexpr uint8_t birdX{5U};
    static constexpr float birdStartY{7.5F};
    static constexpr float ceilingY{0.0F};
    static constexpr float floorY{static_cast<float>(GRID_ROWS - 1U)};
    static constexpr uint8_t gapHeight{5U};
    static constexpr uint8_t gapTopMin{2U};
    static constexpr uint8_t gapTopMax{9U};
    static constexpr uint8_t pipeMax{8U};
    static constexpr int8_t spawnX{static_cast<int8_t>(GRID_COLUMNS - 1U)};
    static constexpr int8_t spawnThresholdX{7};
    static constexpr uint8_t brightnessFull{255U};
    static constexpr uint8_t brightnessHigh{191U};
    static constexpr float gravity{0.22F};
    static constexpr float flapVy{-1.3F};
    static constexpr float vyMin{-2.2F};
    static constexpr float vyMax{2.2F};
    static constexpr float bandUpperOffset{1.6F};
    static constexpr float bandLowerOffset{4.5F};
    static constexpr float riseSuppressVy{-0.3F};
    static constexpr float pipeFloorRiskY{14.0F};
    static constexpr float cruiseFloorRiskY{14.5F};
    static constexpr float cruiseFlapY{9.5F};
    static constexpr float cruiseHoldY{8.0F};
    static constexpr float cruiseFallVy{1.0F};
    static constexpr int8_t nearDxMax{2};
    static constexpr uint8_t physicsInterval{70U};
    static constexpr uint8_t pipeInterval{180U};
    static constexpr uint8_t blinkInterval{200U};
    static constexpr uint8_t blinkTogglesMax{6U};
    static constexpr uint16_t crashPause{900U};
    static constexpr uint8_t predictTicks{5U};

    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        RUNNING,
        CRASH_BLINK,
        CRASH_FALL,
        CRASH_PAUSE,
    };

    struct Pipe
    {
        int8_t x{0};
        uint8_t gy{0U};
    };

    Stage stage{Stage::RUNNING};

    std::array<Pipe, pipeMax> pipes{};
    uint8_t pipeCount{0U};

    float by{birdStartY};
    float vy{0.0F};

    bool blinkVisible{true};
    uint8_t blinkToggles{0U};

    unsigned long lastPhysics{0UL};
    unsigned long lastPipes{0UL};
    unsigned long lastSequence{0UL};

    static bool isSolidRow(const Pipe &pipe, int y);
    static float clampVy(float value);

    void resetGame();
    void spawnPipe();
    void tickPipes();
    void tickPhysics();
    void updateAi();
    [[nodiscard]] const Pipe *findControllingPipe() const;
    [[nodiscard]] bool birdHitsPipe() const;
    void startCrash();
    static void drawPipe(const Pipe &pipe);
    void render() const;

public:
    static constexpr std::string_view name{"Flappy bird"};

    explicit FlappyBirdMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_FLAPPYBIRD
