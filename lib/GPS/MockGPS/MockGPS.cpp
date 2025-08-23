#include "MockGPS.h"




MockGPS::MockGPS(float startLat, float startLon, float moveRate)
    : latitude(startLat), longitude(startLon), moveRate(moveRate) {}

bool MockGPS::init() {
    Logger::logInfo("MockGPS: GPS initialized");
    return true;
}

GPSLocation MockGPS::read() {
    // Simula el movimiento
    latitude += moveRate;
    longitude += moveRate;    
    return { latitude, longitude };    
}

unsigned long MockGPS::getTimestamp() {
    return getMillis();
}

void MockGPS::wakeup() {
    // No es necesario implementar para MockGPS
}

void MockGPS::calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) {
    // Implementación de la fórmula de Haversine
    HaverSine(latitude, longitude, targetLat, targetLon, distance, bearing);
}
