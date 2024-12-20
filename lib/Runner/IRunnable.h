#ifndef IOPERATIONMODE_H
#define IOPERATIONMODE_H


#include "NodeConfiguration/NodeConfiguration.h"
#include "Logger/Logger.h"


class IRunnable {
public:
    // Constructor that receives a reference to the display
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void finish() = 0;

};

#endif // IOPERATIONMODE_H
