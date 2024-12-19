#ifndef ADAFRUIT_DISPLAY_H
#define ADAFRUIT_DISPLAY_H


#include "Adafruit_SSD1306.h"
#include "Display/IDisplay.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D // Address for the OLED display

class AdafruitDisplay : public IDisplay
{
private:
    Adafruit_SSD1306 display;

public:
    AdafruitDisplay();

    void init();

    void print(const std::string &message) override;

    void print(const uint8_t *data, size_t length) override;

    void print(const std::vector<uint8_t> &data) override;

    void print(const String &message) override;

    void print(const char *message) override;

    void clear() override;

};

#endif
