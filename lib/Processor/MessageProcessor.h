#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <Arduino.h>
#include "IProcessor.h"
#include <Adafruit_SSD1306.h>

class MessageProcessor : public IProcessor
{
private:
    Adafruit_SSD1306 *display;

public:
    MessageProcessor(Adafruit_SSD1306 *display) : display(display)
    {
    }

    void processMessage(uint8_t *data, size_t length) override
    {
        // Mostrar mensaje en la pantalla OLED
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->dim(true);
        display->setCursor(0, 0);
        display->print("Message Processor: ");
        display->display();

        for (size_t i = 0; i < length; i++)
        {
            display->print(data[i], HEX);
            display->print(" ");
        }
        display->display();

        // Write the message again to the serial port
        data[1] = Address::BACKEND;
        Serial.write(data, length);
    }
};

#endif