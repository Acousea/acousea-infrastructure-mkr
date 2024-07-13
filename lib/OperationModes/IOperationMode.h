#ifndef IOPERATIONMODE_H
#define IOPERATIONMODE_H

#include <Arduino.h>
#include <../Display/IDisplay.h>
#include "ReportingPeriodManager/ReportingPeriodManager.h"

typedef enum : uint8_t {
    KEEP_CURRENT_MODE = 0,
    LAUNCHING_MODE = 1,
    WORKING_MODE = 2,
    RECOVERY_MODE = 3    
} OPERATION_MODE;



class IOperationMode {
public:
    // Constructor that receives a reference to the display 
    IOperationMode(IDisplay* display) : display(display) {}


    virtual void init(const ReportingPeriods& rp) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual ~IOperationMode() {}

protected:
        IDisplay* display;
};

#endif // IOPERATIONMODE_H
