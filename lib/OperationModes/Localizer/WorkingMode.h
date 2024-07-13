#ifndef LOCALIZERDRIFTINGMODE_H
#define LOCALIZERDRIFTINGMODE_H

#include "../IOperationMode.h"

/**
 * @brief Working mode class (LOCALIZER)
 
 */

class LocalizerWorkingMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
public:
    // Constructor that receives a reference to the display
    LocalizerWorkingMode(IDisplay* display, Router* router, IGPS* gps) 
        : IOperationMode(display), router(router), gps(gps) {}

    void init(const ReportingPeriods& rp) override {        
        display->print("Initializing Working Mode..."); // Cambio de "Initializing Drifting Mode..." a "Initializing Drifting Mode...
        // Código de inicialización específico para el modo drifting
    }

    void run() override {
        display->print("Running Working Mode...");
        // Código específico para ejecutar en modo drifting
        router->relayPorts();
    }

    void stop() override {
        display->print("Stopping Working Mode...");
        // Código de limpieza o parada específica para el modo drifting
    }
};

#endif // DRIFTINGMODE_H
