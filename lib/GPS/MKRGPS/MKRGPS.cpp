#ifdef PLATFORM_ARDUINO

#include "MKRGPS.h"

#include <Arduino.h>
#include <ctime> // Or <time.h> depending on the environment
#include <Arduino_MKRGPS.h>
#include "Logger/Logger.h"

bool MKRGPS::init()
{
    LOG_CLASS_INFO("Initializing GNSS ...");

    if (!GPS.begin())
    {
        LOG_CLASS_INFO("GNSS initialization failed!");
        return false;
    }

    LOG_CLASS_INFO("Awaiting first GNSS fix ...");

    unsigned long startMillis = millis();
    while (!GPS.available() && ((millis() - startMillis) < GNSS_MAX_FIX_TIME_MS))
    {
        delay(500);
    }
    unsigned long endMillis = millis();

    char buffer[50];
    if (endMillis - startMillis >= GNSS_MAX_FIX_TIME_MS)
    {
        LOG_CLASS_ERROR("ERROR: NO GNSS fix after %lu sec\n", (endMillis - startMillis) / 1000);
        return false;
    }
    LOG_CLASS_INFO("Fix: %lu sec\n", (endMillis - startMillis) / 1000);
    return true;
}

GPSLocation MKRGPS::read()
{
    unsigned long startMillis = millis();

    while (!GPS.available() && ((millis() - startMillis) < GNSS_WAIT_TIME_MS))
    {
        delay(500);
    }

    if (GPS.available())
    {
        latitude = GPS.latitude();
        longitude = GPS.longitude();
        return {latitude, longitude};
    }
    else
    {
        LOG_CLASS_INFO("No GNSS data available");
        // Create a GPSLocation with invalid data (-1)
        return {-1, -1};
    }
}

unsigned long MKRGPS::getTimestamp()
{
    return GPS.getTime();
}


void MKRGPS::wakeup()
{
    // Implementar si es necesario
}

void MKRGPS::calculateTrajectory(float targetLat, float targetLon, float& distance, float& bearing)
{
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}

#endif // ARDUINO
