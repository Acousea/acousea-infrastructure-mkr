#ifndef ADAFRUIT_DISPLAY_H
#define ADAFRUIT_DISPLAY_H

#ifdef PLATFORM_ARDUINO

#include "IDisplay.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D // Address for the OLED display

class AdafruitDisplay final : public IDisplay
{
public:
    [[nodiscard]] bool init();

    void print(const uint8_t* data, size_t length) override;

    void print(const char* message) override;

    void clear() override;
};


#endif // ARDUINO

#endif
