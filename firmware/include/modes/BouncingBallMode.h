#pragma once

#if MODE_BOUNCINGBALL

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class BouncingBallMode final : public ModeModule
{
private:
    static constexpr size_t obstaclesMax{14U};
    static constexpr unsigned long physicsInterval{65UL};
    static constexpr unsigned long fadeInterval{40UL};
    static constexpr unsigned long gravityBoostAfter{8000UL};
    static constexpr unsigned long levelTimeout{20000UL};
    static constexpr uint8_t substeps{3U};
    static constexpr uint8_t brightnessObstacle{90U};
    static constexpr uint8_t fadeStep{35U};
    static constexpr float gravity{0.06F};
    static constexpr float gravityBoosted{0.10F};
    static constexpr float radius{0.5F};
    static constexpr float jitter{0.08F};

    struct Obstacle
    {
        uint8_t x{0U};
        uint8_t y{0U};
        uint8_t length{0U};
    };

    enum class Stage : uint8_t
    {
        RUN,
        END,
    };

    Stage stage{Stage::RUN};

    std::array<Obstacle, obstaclesMax> obstacles{};

    size_t obstacleCount{0U};

    uint8_t obstacleBrightness{brightnessObstacle};

    float posX{0.0F};
    float posY{0.0F};
    float velX{0.0F};
    float velY{0.0F};

    unsigned long levelMillis{0UL};
    unsigned long physicsMillis{0UL};
    unsigned long fadeMillis{0UL};

    void generateLevel();
    void spawnBall();
    void collide(uint8_t cellX, uint8_t cellY);
    void tick();
    void render() const;

public:
    static constexpr std::string_view name{"Bouncing ball"};

    explicit BouncingBallMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_BOUNCINGBALL
