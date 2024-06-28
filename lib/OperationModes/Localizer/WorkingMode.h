#ifndef LOCALIZERDRIFTINGMODE_H
#define LOCALIZERDRIFTINGMODE_H

#include "../IOperationMode.h"

/**
 * @brief Working mode class (LOCALIZER)
 
 */

class LocalizerWorkingMode : public IOperationMode {

public:
    // Constructor that receives a reference to the display
    LocalizerWorkingMode(IDisplay* display) : IOperationMode(display) {}

    void init() override {        
        display->print("Initializing Drifting Mode..."); // Cambio de "Initializing Drifting Mode..." a "Initializing Drifting Mode...
        // Código de inicialización específico para el modo drifting
    }

    void run() override {
        display->print("Running Drifting Mode...");
        // Código específico para ejecutar en modo drifting
    }

    void stop() override {
        display->print("Stopping Drifting Mode...");
        // Código de limpieza o parada específica para el modo drifting
    }
};

#endif // DRIFTINGMODE_H
