#ifdef ARDUINO

#include "AdafruitLCBatteryController.h"
#include "Adafruit_LC709203F.h"
#include "Logger/Logger.h"

static Adafruit_LC709203F adafruitLC; // Battery controller

bool AdafruitLCBatteryController::init()
{
    if (!adafruitLC.begin())
    {
        LOG_CLASS_ERROR("Could not find Adafruit LC709203F — make sure a battery is plugged in!");
        return false;
    }

    LOG_CLASS_INFO("Found LC709203F");
    LOG_CLASS_INFO("Version: 0x%X", adafruitLC.getICversion());

    adafruitLC.setThermistorB(3950);
    LOG_CLASS_INFO("Thermistor B = %d", adafruitLC.getThermistorB());

    // Real battery cell is 2200 mAh
    adafruitLC.setPackSize(LC709203F_APA_2000MAH);

    // FIXME: articulate a procedure for servicing this alarm
    adafruitLC.setAlarmVoltage(3.8);
    LOG_CLASS_WARNING("Alarm voltage set to 3.8V — service procedure not defined");

    delay(1000);

    return true;
}

uint8_t AdafruitLCBatteryController::voltageSOC_rounded()
{
    // Convert float to uint8_t getting the integer part
    return (uint8_t)adafruitLC.cellPercent();
}

acousea_BatteryStatus AdafruitLCBatteryController::status()
{
    float v = adafruitLC.cellVoltage();
    if (v >= 4.15) return acousea_BatteryStatus_BATTERY_STATUS_FULL;
    if (v > 3.2) return acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING;
    if (v <= 3.2) return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
    return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
}


float AdafruitLCBatteryController::voltage()
{
    return adafruitLC.cellVoltage();
}

float AdafruitLCBatteryController::temperature()
{
    return adafruitLC.getCellTemperature();
}

#endif
