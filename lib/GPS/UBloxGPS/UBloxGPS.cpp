#include "UBloxGPS.h"




bool UBloxGNSS::init() {
    Logger::logInfo("Initializing GNSS ...");

    if (myGNSS.begin() == false) {
        Logger::logError("GNSS Failed! => u-blox GNSS not detected at default I2C address. Please check wiring");
        return false;
    }

    myGNSS.setI2COutput(COM_TYPE_UBX);
    myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);

    Logger::logInfo("Awaiting first GNSS fix ...");

    bool fixed = false;
    uint32_t beginFix_ms = millis();
    uint8_t fixType = myGNSS.getFixType();
    if ((fixType >= 1) && (fixType <= 4)) fixed = true;

    while (!fixed && ((millis() - beginFix_ms) < GNSS_MAX_FIX_TIME_MS)) {
        fixType = myGNSS.getFixType();
        if ((fixType >= 1) && (fixType <= 4)) fixed = true;
    }

    if (!fixed) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "ERROR: NO GNSS fix after %lu sec\n", (millis() - beginFix_ms) / 1000);
        Logger::logError(buffer);
        return false;
    }

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "GNSS fix took %lu sec\n", (millis() - beginFix_ms) / 1000);
    Logger::logInfo(buffer);
    return true;
}

GPSLocation UBloxGNSS::read() {
    uint32_t beginTime_ms = millis();

    while ((millis() - beginTime_ms) < GNSS_WAIT_TIME_MS) {
        uint8_t fixType = myGNSS.getFixType();
        if ((fixType >= 1) && (fixType <= 4)) break;
    }

    latitude = float(myGNSS.getLatitude() * 1E-7);
    longitude = float(myGNSS.getLongitude() * 1E-7);

    return { latitude, longitude };
}

unsigned long UBloxGNSS::getTimestamp() {
    // Wait for valid time and date. If time is not valid for 10 seconds, return the given time
    unsigned long startWaitTime = millis();
    while ((myGNSS.getTimeValid() == false) || (myGNSS.getDateValid() == false)) {    
        if ((millis() - startWaitTime) > 10000) break;
    }
    return myGNSS.getUnixEpoch();
}

void UBloxGNSS::wakeup() {
    digitalWrite(GNSS_WAKEUP_PIN, HIGH);
    delay(1);
    digitalWrite(GNSS_WAKEUP_PIN, LOW);
}

void UBloxGNSS::calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) {
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}
