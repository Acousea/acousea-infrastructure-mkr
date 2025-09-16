#include "test_main.h"

void test_setup()
{
#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif
    // ENSURE(storageManager, "storageManager");
    ENSURE(display, "display");
    // ENSURE(gps, "gps");
    // ENSURE(rtcController, "rtcController");
    ENSURE(batteryController, "battery");
    // ENSURE(serialPort, "serialPort");
    // ENSURE(loraPort, "loraPort");
    // ENSURE(iridiumPort, "iridiumPort");

#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: Ensured pointers\n");
#endif

#ifdef ARDUINO
    // Inicializa la comunicación serial a 9600 baudios
    serialUSBDisplay.init(9600);
    delay(2000); // Espera a que el monitor serie esté listo
#endif

}

void test_loop()
{
    static unsigned long lastTime = 0;
    // Operate every 30 seconds
    if (getMillis() - lastTime >= 15000 || lastTime == 0)
    {
        // nodeOperationRunner.init();
        // nodeOperationRunner.run();
        const auto percentage = batteryController->percentage();
        const auto status = batteryController->status();
        Logger::logInfo(
            "Battery: " + std::to_string(percentage) + "%, Status: " + std::to_string(static_cast<int>(status))
            );
        lastTime = getMillis();
    }
}

#ifdef ARDUINO

void test_onReceiveWrapper(int packetSize){
    SerialUSB.println("OnReceiveWrapper Callback");
    // realLoraPort.onReceive(packetSize);
}

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler(){
    // mySerial3.IrqHandler();
}

#endif


