#include "dev_dependencies.h"

// #include "../lib/MockLib/include/library.h"

#define ENSURE(ptr, name) do{ if(!(ptr)){ std::fprintf(stderr,"[native] %s is NULL\n", name); return; } }while(0)


#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario

void dev_saveLocalizerConfig();
void dev_saveDrifterConfig();


void dev_setup();
void dev_loop();

#ifdef ARDUINO

void dev_onReceiveWrapper(int packetSize);
// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void dev_SERCOM3_Handler();

#endif
