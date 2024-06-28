#ifndef IOPERATIONMODE_H
#define IOPERATIONMODE_H

#include <Arduino.h>
#include <../Display/IDisplay.h>

class IOperationMode {
public:
    // Constructor that receives a reference to the display 
    IOperationMode(IDisplay* display) : display(display) {}


    virtual void init() = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual ~IOperationMode() {}

protected:
        IDisplay* display;
};

#endif // IOPERATIONMODE_H
