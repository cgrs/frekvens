#pragma once

#if MODE_TETRIS

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class TetrisMode final : public ModeModule
{
private:
    static constexpr uint8_t pieceCount{7U};
    static constexpr uint8_t spawnX{(GRID_COLUMNS - 4U) / 2U};
    static constexpr uint16_t gravityBase{300U};
    static constexpr uint16_t gravityStep{12U};
    static constexpr uint16_t gravityMin{120U};
    static constexpr uint16_t blinkInterval{120U};
    static constexpr uint16_t flashInterval{150U};
    static constexpr uint8_t sequenceToggles{6U};

    // 4x4 bitmasks, one per rotation
    static constexpr std::array<std::array<uint16_t, 4U>, pieceCount> pieces{{
        {0x00F0U, 0x2222U, 0x0F00U, 0x1111U},
        {0x0033U, 0x0033U, 0x0033U, 0x0033U},
        {0x0072U, 0x0262U, 0x0270U, 0x0232U},
        {0x0036U, 0x0462U, 0x0360U, 0x0231U},
        {0x0063U, 0x0264U, 0x0630U, 0x0132U},
        {0x0071U, 0x0226U, 0x0470U, 0x0322U},
        {0x0074U, 0x0622U, 0x0170U, 0x0223U},
    }};
    static constexpr std::array<uint8_t, pieceCount> brightness{255U, 200U, 151U, 100U, 75U, 40U, 10U};

    using Field = std::array<std::array<uint8_t, GRID_COLUMNS>, GRID_ROWS>;

    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        RUNNING,
        CLEARING,
        GAMEOVER,
    };

    Stage stage{Stage::RUNNING};

    Field field{};

    uint8_t pieceType{0U};
    uint8_t pieceRot{0U};
    int8_t pieceX{spawnX};
    int8_t pieceY{0};
    uint8_t targetRot{0U};
    int8_t targetX{spawnX};

    uint16_t dropInterval{gravityBase};
    uint16_t linesTotal{0U};

    std::array<uint8_t, 4U> clearRows{};
    uint8_t clearRowCount{0U};

    uint8_t toggles{0U};
    bool blinkVisible{true};
    bool fieldVisible{true};

    unsigned long lastMillis{0UL};

    static bool collides(uint8_t type, uint8_t rot, int8_t px, int8_t py, const Field &_field);
    static uint8_t columnHeight(const Field &_field, uint8_t col);
    static uint16_t countHoles(const Field &_field);
    static uint16_t countFullRows(const Field &_field);
    static int32_t scorePlacement(const Field &_field, uint16_t holesBefore);

    void simulateDrop(uint8_t type, uint8_t rot, uint8_t left, Field &out) const;
    void planAi();

    void tickRunning();
    void dropStep();
    void lockPiece();
    void startClearing();
    [[nodiscard]] bool isClearingRow(uint8_t y) const;
    void collapse();
    void updateSpeed();
    void spawnPiece();
    void startGameOver();
    void resetGame();

    void render() const;

public:
    static constexpr std::string_view name{"Tetris"};

    explicit TetrisMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_TETRIS
