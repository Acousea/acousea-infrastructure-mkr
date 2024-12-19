#ifndef IOPERATIONMODE_H
#define IOPERATIONMODE_H


#include "NodeConfiguration/NodeConfiguration.h"
#include "IDisplay.h"


class IRunnable {
public:
    // Constructor that receives a reference to the display 
    IRunnable(IDisplay* display) : display(display) {}

    virtual void init() = 0;
    virtual void run() = 0;
    virtual void finish() = 0;
    virtual ~IRunnable() {}

protected:
        IDisplay* display;
};

#endif // IOPERATIONMODE_H
