#pragma once

#if MODE_SPACEINVADERS

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class SpaceInvadersMode final : public ModeModule
{
private:
    static constexpr uint8_t enemyRows{3U};
    static constexpr uint8_t enemyCols{5U};
    static constexpr uint8_t baseRowY{1U};
    static constexpr uint8_t rowSpacing{2U};
    static constexpr uint8_t colStartX{1U};
    static constexpr uint8_t colSpacing{3U};
    static constexpr uint8_t shipY{GRID_ROWS - 1U};
    static constexpr uint8_t shipStartX{(GRID_COLUMNS - 1U) / 2U};
    static constexpr uint8_t bulletSpawnY{shipY - 2U};
    static constexpr uint8_t gameOverY{GRID_ROWS - 2U};

    static constexpr uint8_t brightnessFull{255U};
    static constexpr uint8_t brightnessHigh{191U};
    static constexpr uint8_t brightnessMid{150U};

    static constexpr uint16_t marchInterval{900U};
    static constexpr uint8_t moveInterval{100U};
    static constexpr uint8_t bulletInterval{60U};
    static constexpr uint16_t wavePause{800U};
    static constexpr uint8_t blinkInterval{200U};
    static constexpr uint8_t blinkTogglesMax{8U};

    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        RUNNING,
        GAME_OVER_BLINK,
        WAVE_DELAY,
    };

    Stage stage{Stage::RUNNING};

    std::array<std::array<bool, enemyCols>, enemyRows> alive{};
    int8_t colOffset{0};
    int8_t rowOffset{0};
    int8_t marchDir{1};
    uint16_t waveIntervalMs{marchInterval};
    uint8_t enemiesAlive{0U};

    int8_t shipX{shipStartX};
    int8_t targetX{shipStartX};

    bool bulletActive{false};
    int8_t bulletX{0};
    int8_t bulletY{0};

    bool blinkVisible{true};
    uint8_t blinkToggles{0U};

    unsigned long lastMarch{0UL};
    unsigned long lastMove{0UL};
    unsigned long lastBullet{0UL};
    unsigned long lastSequence{0UL};

    static void plot(int x, int y, uint8_t brightness);

    void resetGame();
    void spawnWave();
    void updateFormation();
    void updateShip();
    void updateBullet();
    void startWaveClear();
    void startGameOver();
    [[nodiscard]] int enemyX(uint8_t col) const;
    [[nodiscard]] int enemyY(uint8_t row) const;
    void render() const;

public:
    static constexpr std::string_view name{"Space invaders"};

    explicit SpaceInvadersMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_SPACEINVADERS
