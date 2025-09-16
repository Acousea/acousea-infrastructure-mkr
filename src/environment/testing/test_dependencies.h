#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include <libraries.h>


// =======================================================
//       COMMON
// =======================================================
// ---- Batería ----
extern MockBatteryController mockBatteryController;
extern IBatteryController* batteryController;

// ---- Display ----
extern IDisplay* display;



// =======================================================
//       ARDUINO BUILD
// =======================================================
#ifdef ARDUINO

// USB Display
extern SerialUSBDisplay serialUSBDisplay;

// =======================================================
//       NATIVE BUILD
// =======================================================
#else // NATIVE


#endif // ARDUINO vs NATIVE

#endif // DEPENDENCIES_H


