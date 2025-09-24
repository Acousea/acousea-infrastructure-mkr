#include "test_main.h"
#include "wiring_private.h"
#include "WVariant.h"


void test_setup(){
#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif
    // ENSURE(storageManager, "storageManager");
    ENSURE(display, "display");
    ENSURE(gps, "gps");
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
    // sercom0.resetUART();
    // sercom0.initUART(UART_INT_CLOCK, SAMPLE_RATE_x16, 9600);
    // sercom0.initFrame(UART_CHAR_SIZE_8_BITS, LSB_FIRST, SERCOM_NO_PARITY, SERCOM_STOP_BIT_1);
    // // sercom0.initPads(UART_TX_PAD_2, SERCOM_RX_PAD_3);
    // sercom0.initPads(UART_TX_PAD_0, SERCOM_RX_PAD_1);
    // sercom0.enableUART();

    ConsoleSerial.begin(9600);

    delay(3000);
    ConsoleSerial.println("Initialized ConsoleSerial at 9600 baud");

    // softwareSerialSercom0.begin(9600);
    // delay(3000);
    //
    // /// PinPeripheral after begin()
    pinPeripheral(PIN_A5, PIO_SERCOM_ALT); // TX
    pinPeripheral(PIN_A6, PIO_SERCOM_ALT); // RX
    pinPeripheral(PIN_SPI_MOSI, PIO_SERCOM); // TX
    pinPeripheral(PIN_SPI_SCK, PIO_SERCOM); // RX
    //
    // softwareSerialSercom0.println("Initialized serialSercom0 at 9600 baud");
    ConsoleSerial.println("Initialized serialSercom0 at 9600 baud");

#endif

    // Logger initialization and configuration
    Logger::initialize(
        display,
        nullptr,
        "log", // MAX 8 chars for 8.3 filenames
        Logger::Mode::SerialOnly
    );
    Logger::logInfo("================ Setting up Node =================");

    // batteryController->init();

    // Initialize the gps
    // gps->init();


#ifdef PLATFORM_HAS_GSM
    // ------------------------ Test GSM Connection ------------------------
    gsmPort.init();

    ConsoleSerial.println("Preparing to send packet using GSM...");
    // MyGSMSSLClient myGsmSslClient;
    // myGsmSslClient.updateCerts(GSM_ROOT_CERTS, std::size(GSM_ROOT_CERTS));
    // myGsmSslClient.listCertificates(CertType::All);

    // Enviar usando tu GsmPort
    // const std::vector<uint8_t> payload = {
    //     0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04,
    //     0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    // };
    // gsmPort.send(payload);
    // GsmPort::testConnection("example.com", 80, "/", false);

    // gsmPort.testConnection("www.google.com", 443, "/", true);
    // gsmPort.testConnection("antapagon.com", 443, "/", true);
    gsmPort.testConnection("test432091-framework22.antapagon.com", 443, "/", true);
    gsmPort.testConnection("www.antapagon.com", 443, "/", true);


    ConsoleSerial.println("Sent packet using GSM");
    // ---------------------------------------------------------------------
#endif
}

// void test_loop(){
//     static unsigned long lastTime = 0;
//     // Operate every 30 seconds
//     if (getMillis() - lastTime >= 15000 || lastTime == 0){
//         // nodeOperationRunner.init();
//         // nodeOperationRunner.run();
//         const auto percentage = batteryController->percentage();
//         const auto status = batteryController->status();
//         Logger::logInfo(
//             "Battery: " + std::to_string(percentage) + "%, Status: " + std::to_string(static_cast<int>(status))
//         );
//         lastTime = getMillis();
//     }
// }
//

void test_loop(){
    static unsigned long lastTime = 0;
    if (getMillis() - lastTime >= 150000 || lastTime == 0){
        // Try to send a packet
        // ConsoleSerial.println("Sending packet...");
        // const std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        // gsmPort.send(data);

        ConsoleSerial.println("Looping...");
        lastTime = getMillis();
    }
}

#ifdef ARDUINO

#if defined(PLATFORM_HAS_LORA)
void test_onReceiveWrapper(int packetSize){
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler(){
    // mySerial3.IrqHandler();
}

void test_SERCOM1_Handler(){
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom1.IrqHandler();
}

void test_SERCOM0_Handler(){
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom0.IrqHandler();
}


#endif
