#include "prod_dependencies.h"
#include "../shared_utils.hpp"

// #include "../lib/MockLib/include/library.h"

#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario

void prod_saveLocalizerConfig();
void prod_saveDrifterConfig();


void prod_setup();
void prod_loop();

#ifdef PLATFORM_ARDUINO

#if defined(PLATFORM_HAS_LORA)
void prod_onReceiveWrapper(int packetSize);
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void prod_SERCOM0_Handler();
void prod_SERCOM1_Handler();
void prod_SERCOM3_Handler();

#endif
