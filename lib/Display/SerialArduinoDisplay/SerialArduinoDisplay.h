#ifndef SERIAL_UART_DISPLAY_H
#define SERIAL_UART_DISPLAY_H

#ifdef ARDUINO

#include <Arduino.h>
#include "IDisplay.h"

class SerialArduinoDisplay : public IDisplay{
public:
    explicit SerialArduinoDisplay(Stream* serial);

    void print(const uint8_t* data, size_t length) override;
    void print(const std::string& message) override;
    void print(const std::vector<uint8_t>& data);
    void print(const char* message) override;
    void clear() override;

private:
    Stream* serialPort;
};

#endif // ARDUINO
#endif // SERIAL_UART_DISPLAY_H
