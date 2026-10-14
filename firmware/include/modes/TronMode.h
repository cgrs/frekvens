#pragma once

#if MODE_TRON

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>

class TronMode final : public ModeModule
{
private:
    static constexpr uint8_t riderCount{2U};
    static constexpr uint8_t moveOptionCount{3U};
    static constexpr uint16_t trailMax{GRID_COLUMNS * GRID_ROWS};
    static constexpr uint8_t stepInterval{220U};
    static constexpr uint8_t flashInterval{120U};
    static constexpr uint8_t flashToggles{6U};
    static constexpr uint8_t rewindInterval{25U};
    static constexpr std::array<uint8_t, riderCount> riderBrightness{255U, 60U};

    enum class Stage : uint8_t // NOLINT(performance-enum-size)
    {
        RUNNING,
        FLASH,
        REWIND,
    };

    struct Rider
    {
        int8_t x{0};
        int8_t y{0};
        int8_t dx{0};
        int8_t dy{0};
    };

    struct MoveOption
    {
        int8_t dx{0};
        int8_t dy{0};
        int8_t tx{0};
        int8_t ty{0};
    };

    Stage stage{Stage::RUNNING};

    std::array<Rider, riderCount> riders{};
    std::array<std::array<uint8_t, GRID_COLUMNS>, GRID_ROWS> trail{};
    std::array<std::array<int8_t, trailMax>, riderCount> histX{};
    std::array<std::array<int8_t, trailMax>, riderCount> histY{};
    std::array<uint16_t, riderCount> histLen{};
    std::array<bool, riderCount> loser{};

    bool blinkVisible{true};
    uint8_t blinkToggles{0U};

    unsigned long lastMillis{0UL};

    [[nodiscard]] bool cellFree(int8_t x, int8_t y) const;
    [[nodiscard]] bool cellFreeForSearch(int8_t x, int8_t y, int8_t excludeAX, int8_t excludeAY, int8_t excludeBX,
                                         int8_t excludeBY) const;
    [[nodiscard]] uint16_t floodFillCount(int8_t startX, int8_t startY, int8_t excludeAX, int8_t excludeAY,
                                          int8_t excludeBX, int8_t excludeBY) const;

    void resetBoard();
    void placeRider(uint8_t idx, int8_t x, int8_t y, int8_t dx, int8_t dy);
    [[nodiscard]] bool chooseMove(uint8_t idx, int8_t &targetX, int8_t &targetY, int8_t otherPendingX,
                                  int8_t otherPendingY) const;
    void commitMove(uint8_t idx, int8_t targetX, int8_t targetY);
    void tickRiders();
    void endRound(bool loserOne, bool loserTwo);
    void rewindTick();
    void render() const;

public:
    static constexpr std::string_view name{"Tron"};

    explicit TronMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_TRON
