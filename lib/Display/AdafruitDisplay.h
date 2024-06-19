#ifndef ADAFRUIT_DISPLAY_H
#define ADAFRUIT_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "IDisplay.h"

class AdafruitDisplay : public IDisplay {
private:
    Adafruit_SSD1306* display;

public:
    AdafruitDisplay(Adafruit_SSD1306* display) : display(display) {}
    
    void init() {
        if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
            SerialUSB.println(F("SSD1306 allocation failed"));
            while (true) {delay(10);};
        }
        display->display();
        delay(2000); // Pausa inicial para la pantalla
        display->clearDisplay();
    }

    void print(const uint8_t* data, size_t length) override {
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);

        for (size_t i = 0; i < length; i++) {
            display->print(data[i], HEX);
            display->print(" ");
        }
        display->display();
    }

    virtual void print(const std::vector<uint8_t> &data){
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);

        for (size_t i = 0; i < data.size(); i++) {
            display->print(data[i], HEX);
            display->print(" ");
        }
        display->display();
    }

    void print(const String& message) override {
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        display->print(message);
        display->display();
    }

    void print(int value) override {
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        display->print(value);
        display->display();
    }
};

#endif
