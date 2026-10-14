#pragma once

#if MODE_PONGCLOCK

#include "config/constants.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <ctime>

class PongClockMode final : public ModeModule
{
private:
    static constexpr std::array<uint16_t, 10U> digits{
        0b111101101101111U, // 0
        0b010110010010111U, // 1
        0b111001111100111U, // 2
        0b111001111001111U, // 3
        0b101101111001001U, // 4
        0b111100111001111U, // 5
        0b111100111101111U, // 6
        0b111001001001001U, // 7
        0b111101111101111U, // 8
        0b111101111001111U, // 9
    };

    static constexpr uint8_t fieldTop{6U};
    static constexpr uint8_t fieldHeight{static_cast<uint8_t>(GRID_ROWS - fieldTop)};
    static constexpr unsigned long stepInterval{100UL};
    static constexpr unsigned long celebrationDuration{2000UL};

    tm local{};

    bool ballVisible{true};
    bool celebrating{false};
    bool scoreHour{false};
    bool scoreMinute{false};

    int8_t direction{-1};

    int shownHour{24};
    int shownMinute{60};

    uint8_t ballColumn{GRID_COLUMNS - 2U};
    uint8_t paddleLeft{fieldHeight / 2U};
    uint8_t paddleRight{fieldHeight / 2U};
    uint8_t targetLeft{fieldHeight / 2U};
    uint8_t targetRight{fieldHeight / 2U};

    float ballRow{fieldHeight / 2.0F};
    float ballVelocity{0.0F};

    unsigned long celebrationEnd{0UL};
    unsigned long lastBlink{0UL};
    unsigned long lastStep{0UL};

    static void advance(float &row, float &velocity);

    void drawDigit(uint16_t bitmap, uint8_t x) const;
    void drawDigits() const;
    void drawField() const;
    void launch();
    void plan();
    void step();

    [[nodiscard]] uint8_t predictRow() const;

public:
    static constexpr std::string_view name{"Pong clock"};

    explicit PongClockMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};

#endif // MODE_PONGCLOCK
