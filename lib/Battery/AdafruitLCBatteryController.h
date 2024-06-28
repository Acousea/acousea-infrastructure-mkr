#ifndef ADAFRUITLC_MANAGER_H
#define ADAFRUITLC_MANAGER_H

#include <Arduino.h>
#include <Adafruit_LC709203F.h>
#include "IBattery.h"

class AdafruitLCBatteryController : public IBattery {
public:
    bool init() override {       
        if (!adafruitLC.begin()) {
            SerialUSB.println(F("Could not find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
            return false;
        }
        SerialUSB.println(F("Found LC709203F"));
        SerialUSB.print("Version: 0x"); SerialUSB.println(adafruitLC.getICversion(), HEX);

        adafruitLC.setThermistorB(3950);
        SerialUSB.print("Thermistor B = "); SerialUSB.println(adafruitLC.getThermistorB());

        // Real battery cell is 2200 mAh
        adafruitLC.setPackSize(LC709203F_APA_2000MAH);

        // FIXME: articulate a procedure for servicing this alarm 
        adafruitLC.setAlarmVoltage(3.8);

        // Esperar un segundo antes de verificar el estado
        delay(1000);

        return true;
    }

    uint8_t percentage() override { // Convert float to uint8_t getting the integer part        
        return (uint8_t) adafruitLC.cellPercent();
    }

    float voltage() {
        return adafruitLC.cellVoltage();
    }

    float temperature() {
        return adafruitLC.getCellTemperature();
    }

private:
    Adafruit_LC709203F adafruitLC; // Battery controller
};

#endif // AdafruitLC_MANAGER_H