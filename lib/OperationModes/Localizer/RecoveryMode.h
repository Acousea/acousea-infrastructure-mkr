#ifndef LOCALIZERRECOVERYMODE_H
#define LOCALIZERRECOVERYMODE_H


#include "../IOperationMode.h"

/**
 * @brief Recovery mode class (DRIFTER)
 * The launching mode is the last mode of operation, immediatly after the drifting mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 *  - The router is used to send the current location data from the GPS to the localizer. 
 *  - The mode a set duration of `LAUNCH_MODE_CYCLES`, after which it will transition to the DRIFTING MODE.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_LAUNCH_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_LAUNCH_SEC`.  
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 */


class LocalizerRecoveryMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;

public:
    // Constructor that receives a reference to the display
    LocalizerRecoveryMode(IDisplay* display, Router* router, IGPS* gps) 
        : IOperationMode(display), router(router), gps(gps) {}

    void init(const ReportingPeriods& rp) override {        
        display->print("Initializing Recovery Mode..."); // Cambio de "Initializing Recovery Mode..." a "Initializing Recovery Mode...
        // Código de inicialización específico para el modo recovery
    }

    void run() override {
        display->print("Running Recovery Mode...");
        // Código específico para ejecutar en modo recovery
        router->relayPorts();
    }

    void stop() override {
        display->print("Stopping Recovery Mode...");
        // Código de limpieza o parada específica para el modo recovery
    }
};

#endif // RECOVERYMODE_H
