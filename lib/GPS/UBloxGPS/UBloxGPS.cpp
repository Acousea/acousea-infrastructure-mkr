#ifdef ARDUINO

#include "UBloxGPS.h"

#include <Logger/Logger.h>
#include "SparkFun_u-blox_GNSS_Arduino_Library.h"



static SFE_UBLOX_GNSS myGNSS;

// Definición de los posibles tipos de fix GNSS
enum GNSSFixType : uint8_t
{
    NO_FIX = 0, // No fix
    DEAD_RECKONING_ONLY = 1, // Solo Dead Reckoning
    FIX_2D = 2, // Fix 2D
    FIX_3D = 3, // Fix 3D
    GNSS_DEAD_RECKONING = 4, // GNSS + Dead Reckoning
    TIME_ONLY_FIX = 5 // Solo tiempo válido (sin posición)
};

const char* fixTypeToString(GNSSFixType fixType)
{
    switch (fixType)
    {
    case NO_FIX: return "No Fix";
    case DEAD_RECKONING_ONLY: return "Dead Reckoning only";
    case FIX_2D: return "2D Fix";
    case FIX_3D: return "3D Fix";
    case GNSS_DEAD_RECKONING: return "GNSS + Dead Reckoning";
    case TIME_ONLY_FIX: return "Time only";
    default: return "Unknown";
    }
}

// ---------------------------
// Método de configuración GNSS
// ---------------------------
bool UBloxGNSS::config()
{
    Logger::logInfo(getClassNameString() + "Configuring GNSS ...");

    // Salida solo UBX en I2C (interno a tu micro)
    myGNSS.setI2COutput(COM_TYPE_UBX);

    // Habilitar NMEA en UART1
    myGNSS.setUART1Output(COM_TYPE_NMEA); // Activar salida NMEA en UART1
    myGNSS.setSerialRate(9600); // Velocidad 9600 bps

    // Activar solo frases mínimas para posicionamiento (GGA = fix, RMC = tiempo/velocidad)
    myGNSS.enableNMEAMessage(UBX_NMEA_GGA, COM_PORT_UART1);
    myGNSS.enableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);

    // Opcional: desactivar el resto para evitar ruido y ahorrar ancho de banda
    myGNSS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_UART1);
    myGNSS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_UART1);
    myGNSS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_UART1);
    myGNSS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_UART1);

    // Constelaciones recomendadas (puedes ajustar según aplicación)
    myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
    myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GALILEO);
    myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);

    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);
    myGNSS.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);

    // Modelo dinámico: PORTABLE es versátil, STATIONARY si es una boya fija
    if (!myGNSS.setDynamicModel(DYN_MODEL_PORTABLE))
    {
        Logger::logError(getClassNameString() + "Failed to set dynamic model");
        return false;
    }

    // Tasa de actualización (Hz). 1 Hz ahorra energía, 5 Hz es más rápido.
    if (!myGNSS.setNavigationFrequency(1))
    {
        Logger::logError(getClassNameString() + "Failed to set navigation frequency");
        return false;
    }

    // Power save mode (si tu módulo lo soporta)
    if (myGNSS.powerSaveMode())
    {
        Logger::logInfo(getClassNameString() + "Power Save Mode enabled");
    }
    else
    {
        Logger::logError(getClassNameString() + "Power Save Mode not supported / failed");
    }

    // Guardar config en memoria
    const bool saved = myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT) &&
        myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_NAVCONF);

    if (saved)
    {
        Logger::logInfo(getClassNameString() + "Configuration saved");
    }
    else
    {
        Logger::logError(getClassNameString() + "Configuration save failed");
    }

    return saved;
}

bool UBloxGNSS::init()
{
    Logger::logInfo(getClassNameString() + "Initializing GNSS ...");

    if (myGNSS.begin() == false)
    {
        Logger::logError(getClassNameString() +
            "GNSS Failed! => u-blox GNSS not detected at default I2C address. Please check wiring");
        return false;
    }

    if (!config())
    {
        Logger::logError(getClassNameString() + "GNSS configuration failed");
        return false;
    }
    Logger::logInfo(getClassNameString() + "GNSS initialized");
    Logger::logInfo(getClassNameString() + "Awaiting first GNSS fix ...");

    bool fixed = false;
    const uint32_t beginFix_ms = millis();

    auto fixType = static_cast<GNSSFixType>(myGNSS.getFixType());
    if (fixType != NO_FIX && fixType != TIME_ONLY_FIX)
    {
        fixed = true;
    }
    while (!fixed && ((millis() - beginFix_ms) < GNSS_MAX_FIX_TIME_MS))
    {
        fixType = static_cast<GNSSFixType>(myGNSS.getFixType());
        if (fixType != NO_FIX && fixType != TIME_ONLY_FIX)
        {
            fixed = true;
        }
    }

    if (!fixed)
    {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "ERROR: NO GNSS fix after %lu sec\n", (millis() - beginFix_ms) / 1000);
        Logger::logError(getClassNameString() + buffer);
        return false;
    }
    Logger::logInfo(getClassNameString() + "Fix type: " + fixTypeToString(fixType));


    char buffer[50];
    snprintf(buffer, sizeof(buffer), "GNSS fix took %lu sec\n", (millis() - beginFix_ms) / 1000);
    Logger::logInfo(getClassNameString() + buffer);
    return true;
}

GPSLocation UBloxGNSS::read()
{
    const uint32_t beginTime_ms = millis();

    while ((millis() - beginTime_ms) < GNSS_WAIT_TIME_MS)
    {
        const auto fixType = static_cast<GNSSFixType>(myGNSS.getFixType());
        if (fixType != NO_FIX && fixType != TIME_ONLY_FIX)
        {
            break;
        }
    }

    latitude = float(myGNSS.getLatitude() * 1E-7);
    longitude = float(myGNSS.getLongitude() * 1E-7);

    return {latitude, longitude};
}

unsigned long UBloxGNSS::getTimestamp()
{
    // Wait for valid time and date. If time is not valid for 10 seconds, return the given time
    unsigned long startWaitTime = millis();
    while ((myGNSS.getTimeValid() == false) || (myGNSS.getDateValid() == false))
    {
        if ((millis() - startWaitTime) > GNSS_MAX_TIMESTAMP_WAIT_TIME_MS) break;
    }
    return myGNSS.getUnixEpoch();
}

void UBloxGNSS::wakeup()
{
    digitalWrite(GNSS_WAKEUP_PIN, HIGH);
    delay(1);
    digitalWrite(GNSS_WAKEUP_PIN, LOW);
}

void UBloxGNSS::calculateTrajectory(float targetLat, float targetLon, float& distance, float& bearing)
{
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}


#endif // ARDUINO
