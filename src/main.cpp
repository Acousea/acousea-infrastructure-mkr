#include "libraries.h"
#include "config.h"

#include "../lib/SDLogger/SDLogger.h"

SDLogger sdLogger;

void setup() {
    // Inicialización de sdLogger
    SerialUSB.begin(9600);
    SerialUSB.println("Initializing SDLogger...");
    // sdLogger.init();
    rtc.begin();
}

void loop() {
    
}