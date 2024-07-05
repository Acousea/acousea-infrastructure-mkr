#ifndef LOCALIZERLAUNCHINGMODE_H
#define LOCALIZERLAUNCHINGMODE_H

#include "../IOperationMode.h"
#include "../Router/Router.h"
#include "../GPS/IGPS.h"

/**
 * @brief Launching mode class (LOCALIZER)
 * The launching mode is the second mode of operation, immediatly after the ignition mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 * The localizer will receive the location data from the GPS and send it to the backend.
 */

class LocalizerLaunchingMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
public:
    // Constructor that receives a reference to the display
    LocalizerLaunchingMode(IDisplay* display, Router* router, IGPS* gps) 
    : IOperationMode(display), router(router), gps(gps) {}

    void init() override {        
        display->print("Initializing Launching Mode..."); // Cambio de "Initializing Launching Mode..." a "Initializing Launching Mode...
        // Código de inicialización específico para el modo launching

    }

    void run() override {
        display->print("Running Launching Mode...");        
        router->relayPorts();  
    }

    void stop() override {
        display->print("Stopping Launching Mode...");
        // Código de limpieza o parada específica para el modo launching
    }
};

#endif // LAUNCHINGMODE_H
