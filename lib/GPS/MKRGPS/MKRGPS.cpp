#ifdef PLATFORM_ARDUINO

#include "MKRGPS.h"

#include <Arduino.h>
#include <ctime> // Or <time.h> depending on the environment
#include <Arduino_MKRGPS.h>

#include "ErrorHandler/ErrorHandler.h"
#include "Logger/Logger.h"
#include "wait/WaitFor.hpp"

bool MKRGPS::init() // NO LOGGER CALLS HERE (INIT PHASE)
{
    SerialUSB.println(String(getClassNameCString()) + "Initializing GNSS ...");


    if (!GPS.begin())
    {
        ERROR_HANDLE_CLASS("GNSS initialization failed!");
        return false;
    }

    SerialUSB.println(String(getClassNameCString()) + "Awaiting first GNSS fix ...");


    const unsigned long waitedForMs = waitForOrUntil(
        GNSS_MAX_FIX_TIME_MS,
        [&] { return GPS.available(); } // stopIfTrue
    );

    char buffer[50];
    if (waitedForMs >= GNSS_MAX_FIX_TIME_MS)
    {
        ERROR_HANDLE_CLASS("ERROR: NO GNSS fix after %lu sec\n", waitedForMs / 1000);
        return false;
    }

    SerialUSB.println(
        String(getClassNameCString()) + " GNSS fix took " + String((waitedForMs) / 1000) + " sec"
    );

    return true;
}

GPSLocation MKRGPS::read()
{

    waitForOrUntil(
        GNSS_MAX_FIX_TIME_MS,
        [&] { return GPS.available(); } // stopIfTrue
    );

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
