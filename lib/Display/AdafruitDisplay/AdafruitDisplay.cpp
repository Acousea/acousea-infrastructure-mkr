#ifdef ARDUINO
#include "AdafruitDisplay.h"

#include <Arduino.h>
#include "Adafruit_SSD1306.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void AdafruitDisplay::init()
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


void AdafruitDisplay::print(const uint8_t* data, size_t length)
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

void AdafruitDisplay::print(const char* message)
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(message);
    display.display();
}

void AdafruitDisplay::clear()
{
    display.clearDisplay();
    display.display();
}

#endif
