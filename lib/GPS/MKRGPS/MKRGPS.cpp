#include "MKRGPS.h"


bool MKRGPS::init() {
    Logger::logInfo("Initializing GNSS ...");

    if (!GPS.begin()) {
        Logger::logInfo("GNSS initialization failed!");
        return false;
    }

    Logger::logInfo("Awaiting first GNSS fix ...");

    unsigned long startMillis = millis();
    while (!GPS.available() && ((millis() - startMillis) < GNSS_MAX_FIX_TIME_MS)) {
        delay(500);
    }
    unsigned long endMillis = millis();

    char buffer[50];
    if (endMillis - startMillis >= GNSS_MAX_FIX_TIME_MS) {
        snprintf(buffer, sizeof(buffer), "ERROR: NO GNSS fix after %lu sec\n", (endMillis - startMillis) / 1000);
        Logger::logInfo(buffer);
        return false;
    }
    snprintf(buffer, sizeof(buffer), "Fix: %lu sec\n", (endMillis - startMillis) / 1000);
    Logger::logInfo(buffer);
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
        Logger::logInfo("No GNSS data available");
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
