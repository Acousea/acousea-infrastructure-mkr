#ifdef ARDUINO

#include "AdafruitLCBatteryController.h"

bool AdafruitLCBatteryController::init(){
    if (!adafruitLC.begin()){
        SerialUSB.println(F("Could not find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
        return false;
    }
    SerialUSB.println(F("Found LC709203F"));
    SerialUSB.print("Version: 0x");
    SerialUSB.println(adafruitLC.getICversion(), HEX);

    adafruitLC.setThermistorB(3950);
    SerialUSB.print("Thermistor B = ");
    SerialUSB.println(adafruitLC.getThermistorB());

    // Real battery cell is 2200 mAh
    adafruitLC.setPackSize(LC709203F_APA_2000MAH);

    // FIXME: articulate a procedure for servicing this alarm
    adafruitLC.setAlarmVoltage(3.8);

    // Esperar un segundo antes de verificar el estado
    delay(1000);

    return true;
}

uint8_t AdafruitLCBatteryController::voltageSOC_rounded(){
    // Convert float to uint8_t getting the integer part
    return (uint8_t)adafruitLC.cellPercent();
}

acousea_BatteryStatus AdafruitLCBatteryController::status(){
    float v = adafruitLC.cellVoltage();
    if (v >= 4.15) return acousea_BatteryStatus_BATTERY_STATUS_FULL;
    if (v > 3.2) return acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING;
    if (v <= 3.2) return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
    return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
}


float AdafruitLCBatteryController::voltage(){
    return adafruitLC.cellVoltage();
}

float AdafruitLCBatteryController::temperature(){
    return adafruitLC.getCellTemperature();
}

#endif
