#include "dev_dependencies.h"
#include "../shared_utils.hpp"

// #include "../lib/MockLib/include/library.h"

#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario

void dev_saveLocalizerConfig();
void dev_saveDrifterConfig();


void dev_setup();
void dev_loop();

#ifdef ARDUINO

#if defined(PLATFORM_HAS_LORA)
void dev_onReceiveWrapper(int packetSize);
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void dev_SERCOM0_Handler();
void dev_SERCOM1_Handler();
void dev_SERCOM3_Handler();

#endif
