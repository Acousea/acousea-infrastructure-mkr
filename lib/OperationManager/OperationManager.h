#ifndef OPERATION_MANAGER_H
#define OPERATION_MANAGER_H

#include <Arduino.h>
#include "IOperationMode.h"


class OperationManager {
private:
    OPERATION_MODE currentMode;    
    const uint8_t IGNITION_CYCLES = 1;
    const uint8_t LAUNCHING_CYCLES = 3;   
    uint8_t cycleCounter = 0;

public:
    OperationManager() : currentMode(WORKING_MODE) {}

    // Calling setMode on KEEP_CURRENT_MODE will keep the current mode    
    void setMode(OPERATION_MODE mode) {        
        if (mode == KEEP_CURRENT_MODE) {
            return;
        }
        currentMode = mode;
    }

    OPERATION_MODE getMode() const {
        return currentMode;
    }

    void nextCycle() {
        cycleCounter++;
        if (cycleCounter == IGNITION_CYCLES) {            
            setMode(LAUNCHING_MODE);
        }
        if (cycleCounter > LAUNCHING_CYCLES) {            
            setMode(WORKING_MODE);
        }
    }
};

#endif // OPERATION_MANAGER_H
