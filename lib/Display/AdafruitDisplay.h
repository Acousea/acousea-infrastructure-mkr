#ifndef ADAFRUIT_DISPLAY_H
#define ADAFRUIT_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "IDisplay.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D // Address for the OLED display

class AdafruitDisplay : public IDisplay
{
private:
    Adafruit_SSD1306 display;

public:
    AdafruitDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

    void init()
    {
        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        {
            SerialUSB.println(F("SSD1306 allocation failed"));
            while (true)
            {
                delay(10);
            };
        }
        display.display();
        delay(2000); // Pausa inicial para la pantalla
        display.clearDisplay();
    }

    void print(const uint8_t *data, size_t length) override
    {        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);

        for (size_t i = 0; i < length; i++)
        {
            display.print(data[i], HEX);
            display.print(" ");
        }
        display.display();
    }

    void print(const std::vector<uint8_t> &data) override
    {        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);

        for (size_t i = 0; i < data.size(); i++)
        {
            display.print(data[i], HEX);
            display.print(" ");
        }
        display.display();
    }

    void print(const String &message) override
    {        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print(message);
        display.display();
    }

    void print(const char *message) override
    {        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print(message);
        display.display();
    }

    void clear() override
    {
        display.clearDisplay();
        display.display();
    }

};

#endif
