#include "dependencies.h"

#define DRIFTER_MODE 0
#define LOCALIZER_MODE 1
// #define MODE LOCALIZER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario
#define MODE DRIFTER_MODE // Cambiar a DRIFTER_MODE o LOCALIZER_MODE según sea necesario


void setupDrifter()
{
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();

    // Inicializa el administrador de la tarjeta SD
    sdManager.begin();
    
     // Reset the reporting periods to the default values in the config file
    reportingPeriodManager.reset();

    // Inicializa el administrador de periodos de reporte
    reportingPeriodManager.begin();

    // Initialize operation modes with the reporting periods for drifter    
    drifterLaunchingMode.init(reportingPeriodManager.getReportingPeriods("Launching"));
    drifterWorkingMode.init(reportingPeriodManager.getReportingPeriods("Working"));
    drifterRecoveryMode.init(reportingPeriodManager.getReportingPeriods("Recovering"));

    // Inicializa el comunicador Serial
    serialPort.init();

    // Inicializa el administrador de energía
    adafruitLCBatteryController.init();

    // Inicializa el comunicador LoRa
    loraPort.init();

    // Inicializa el comunicador Iridium
    iridiumPort.init();
}

void operateDrifter()
{
    SerialUSB.println("Operating Drifter...");

    // Check if reporting periods configuration has changed
    if (reportingPeriodManager.isNewConfigAvailable()){
        SerialUSB.println("New reporting periods configuration available");        
        drifterLaunchingMode.init(reportingPeriodManager.getReportingPeriods("Launching"));
        drifterWorkingMode.init(reportingPeriodManager.getReportingPeriods("Working"));
        drifterRecoveryMode.init(reportingPeriodManager.getReportingPeriods("Recovering"));
    }
    
    switch (operationManager.getMode())
    {
    case LAUNCHING_MODE:
        drifterLaunchingMode.run();
        break;
    case WORKING_MODE:
        drifterWorkingMode.run();
        break;
    case RECOVERY_MODE:
        drifterRecoveryMode.run();
        break;
    default:
        SerialUSB.println("Unknown operation mode");
        operationManager.setMode(LAUNCHING_MODE);
        break;
    }
}

void setupLocalizer()
{

    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);

    // Inicializa la pantalla Adafruit
    // adafruitDisplay.init();

    // Inicializa el comunicador Serial
    serialPort.init();

    // Inicializa el administrador de energía
    pmicBatteryController.init();

    // Inicializa el comunicador LoRa
    loraPort.init();

    // Inicializa el comunicador Iridium
    iridiumPort.init();
}

void operateLocalizer()
{
    SerialUSB.println("Operating Localizer...");

    // Check if reporting periods configuration has changed
    if (reportingPeriodManager.isNewConfigAvailable()){
        SerialUSB.println("New reporting periods configuration available");        
        localizerLaunchingMode.init(reportingPeriodManager.getReportingPeriods("Launching"));
        localizerWorkingMode.init(reportingPeriodManager.getReportingPeriods("Working"));
        localizerRecoveryMode.init(reportingPeriodManager.getReportingPeriods("Recovering"));
    }

    switch (operationManager.getMode())
    {
    case LAUNCHING_MODE:
        localizerLaunchingMode.run();
        break;
    case WORKING_MODE:
        localizerWorkingMode.run();
        break;
    case RECOVERY_MODE:
        localizerRecoveryMode.run();
        break;
    default:
        SerialUSB.println("Unknown operation mode");
        operationManager.setMode(LAUNCHING_MODE);
        break;
    }
}

void setup()
{
#if MODE == DRIFTER_MODE
    setupDrifter();
#elif MODE == LOCALIZER_MODE
    setupLocalizer();
#endif
}

void loop()
{
    static long lastTime = 0;
    // Operate every 1 second
    if (millis() - lastTime >= 1000)
    {
        lastTime = millis();
#if MODE == DRIFTER_MODE
        operateDrifter();
#elif MODE == LOCALIZER_MODE
        operateLocalizer();
#endif
    }
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}

void onReceiveWrapper(int packetSize)
{
    SerialUSB.println("OnReceiveWrapper Callback");
    loraPort.onReceive(packetSize);
}