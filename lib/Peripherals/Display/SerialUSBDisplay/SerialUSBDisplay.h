#ifndef SERIAL_USB_DISPLAY_H
#define SERIAL_USB_DISPLAY_H

#include "Display/IDisplay.h"

class SerialUSBDisplay : public IDisplay {
public:
    void init(int baudRate = 9600);

    void print(const uint8_t* data, size_t length) override;

    void print(const std::string &message) override;

    virtual void print(const std::vector<uint8_t>& data);

    void print(const String& message) override;

   void print(const char *message) override;

    void clear() override;
        
};

#endif
