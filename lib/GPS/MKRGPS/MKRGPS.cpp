#ifdef ARDUINO

#include "MKRGPS.h"

#include <Arduino.h>
#include <ctime> // Or <time.h> depending on the environment
#include <Arduino_MKRGPS.h>
#include "Logger/Logger.h"

bool MKRGPS::init() {
    Logger::logInfo(getClassNameString() + "Initializing GNSS ...");

    if (!GPS.begin()) {
        Logger::logInfo(getClassNameString() + "GNSS initialization failed!");
        return false;
    }

    Logger::logInfo(getClassNameString() + "Awaiting first GNSS fix ...");

    unsigned long startMillis = millis();
    while (!GPS.available() && ((millis() - startMillis) < GNSS_MAX_FIX_TIME_MS)) {
        delay(500);
    }
    unsigned long endMillis = millis();

    char buffer[50];
    if (endMillis - startMillis >= GNSS_MAX_FIX_TIME_MS) {
        snprintf(buffer, sizeof(buffer), "ERROR: NO GNSS fix after %lu sec\n", (endMillis - startMillis) / 1000);
        Logger::logError(getClassNameString() + buffer);
        return false;
    }
    snprintf(buffer, sizeof(buffer), "Fix: %lu sec\n", (endMillis - startMillis) / 1000);
    Logger::logInfo(getClassNameString() + buffer);
    return true;

}

GPSLocation MKRGPS::read() {
    unsigned long startMillis = millis();

    while (!GPS.available() && ((millis() - startMillis) < GNSS_WAIT_TIME_MS)) {
        delay(500);
    }

    if (GPS.available()) {
        latitude = GPS.latitude();
        longitude = GPS.longitude();
        return {latitude, longitude};
    } else {
        Logger::logInfo(getClassNameString() + "No GNSS data available");
        // Create a GPSLocation with invalid data (-1)
        return {-1, -1};
    }
}

unsigned long MKRGPS::getTimestamp() {
    return GPS.getTime();
}


void MKRGPS::wakeup() {
    // Implementar si es necesario
}

void MKRGPS::calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) {
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}

#endif // ARDUINO