#include "MockGPS.h"


MockGPS::MockGPS(float startLat, float startLon, float moveRate)
    : latitude(startLat), longitude(startLon), moveRate(moveRate){
}

bool MockGPS::init(){
    Logger::logInfo("MockGPS: GPS initialized");
    return true;
}

GPSLocation MockGPS::read(){
    // Simula el movimiento
    latitude += moveRate;
    longitude += moveRate;
    return {latitude, longitude};
}

#ifdef ARDUINO
unsigned long MockGPS::getTimestamp() {
    // Segundos desde el arranque
    return getMillis() / 1000UL;
}
#else  // Nativo
#include <chrono>
unsigned long MockGPS::getTimestamp() {
    using namespace std::chrono;
    // Epoch real (Unix time, segundos)
    return static_cast<unsigned long>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count()
    );
}
#endif

void MockGPS::calculateTrajectory(float targetLat, float targetLon, float& distance, float& bearing){
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}

void MockGPS::wakeup(){
    // No es necesario implementar para MockGPS
    Logger::logInfo("MockGPS: Wakeup called (no action taken)");
}
