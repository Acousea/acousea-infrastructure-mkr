#include "test_main.h"

#include <string>

// #include "WVariant.h"

#include "Ports/GSM/UBlox201/UBlox201_GSMSSLClient.hpp"
#include "TaskScheduler/LambdaTask.hpp"

#ifdef PLATFORM_ARDUINO
void test_solar_x_battery_controller()
{
    // Actualiza cálculos internos
    // solarXBatteryController.sync();

    const auto voltageSOCAcc = solarXBatteryController.voltageSOC_accurate();
    const auto voltageSOCRnd = solarXBatteryController.voltageSOC_rounded();

    const auto coulombSOCAcc = solarXBatteryController.coulombSOC_accurate();
    const auto coulombSOCRnd = solarXBatteryController.coulombSOC_rounded();

    const auto combinedSOCAcc = solarXBatteryController.combinedSOC_accurate();
    const auto combinedSOCRnd = solarXBatteryController.combinedSOC_rounded();

    const auto batteryVoltageVolts = solarXBatteryController.batteryVolts();
    const auto panelVoltageVolts = solarXBatteryController.panelVolts();

    const auto batteryCurrentAmps = solarXBatteryController.batteryCurrentAmp();
    const auto panelCurrentAmps = solarXBatteryController.panelCurrentAmp();
    const auto netPowerWatts = solarXBatteryController.netPowerConsumptionWatts();

    const auto batteryStatus = solarXBatteryController.status();

    LOG_INFO("[TEST_SOLARX_BATTERY_CONTROLLER_CC],%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%.d,%.2f,%.2f,%.2f,%d",
             batteryVoltageVolts,
             panelVoltageVolts,
             batteryCurrentAmps,
             panelCurrentAmps,
             netPowerWatts,
             voltageSOCRnd,
             coulombSOCRnd,
             combinedSOCRnd,
             voltageSOCAcc,
             coulombSOCAcc,
             combinedSOCAcc,
             static_cast<int>(batteryStatus));


    LOG_FREE_MEMORY("[FREEMEM]");
}


#ifdef PLATFORM_HAS_GSM
void test_gsm_initialization()
{
    // ------------------------ Test GSM Connection ------------------------
    // gsmPort.init();
    // LOG_FREE_MEMORY("[MAIN] After GSM init");
    LOG_FREE_MEMORY("[FREEMEM] Before listing certs");
    UBlox201_GSMSSLClient myGsmSslClient;
    UBlox201_GSMSSLClient::setModemDebug();
    const auto certs = myGsmSslClient.listCertificates(CertType::All);
    GsmMQTTPort::printCertificates(certs);
    // myGsmSslClient.updateCerts(GSM_ROOT_CERTS, std::size(GSM_ROOT_CERTS));
    LOG_FREE_MEMORY("[MAIN] After listing certs");

    // Enviar usando tu GsmPort
    // const std::vector<uint8_t> payload = {
    //     0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04,
    //     0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    // };

    const std::vector<uint8_t> payload = {
        // Error payload example
        0x08, 0x63, 0x12, 0x04, 0x08, 0x65, 0x18, 0x07,
        0x32, 0x1D, 0x0A, 0x1B, 0x54, 0x68, 0x69, 0x73,
        0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x74, 0x65,
        0x73, 0x74, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72,
        0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74
    };

    // gsmPort.send(payload);
    // GsmPort::testConnection("example.com", 80, "/", false);

    // myGsmSslClient.testConnection("www.google.com", 443, "/");
    // myGsmSslClient.testConnection("antapagon.com", 443, "/");
    const auto connOk = myGsmSslClient.testTLSConnection("test432091-framework22.antapagon.com", 443, "/");

    LOG_INFO("GSM TLS connection test result: %s", connOk ? "SUCCESS" : "FAILURE");
    // myGsmSslClient.testConnection("www.antapagon.com", 443, "/", true);


    ConsoleSerial.println("Sent packet using GSM");
    // ---------------------------------------------------------------------
}


void test_gsm_sending_packets()
{
    // Try to send a packet
    const auto packets = gsmPort.read();
    if (packets.empty())
    {
        ConsoleSerial.println("No packets received.");
    }
    else
    {
        ConsoleSerial.println("Received packets:");
        for (const auto& packet : packets)
        {
            const auto hexString = Logger::vectorToHexString(packet.data(), packet.size());
            ConsoleSerial.println(hexString.c_str());
        }
    }

    ConsoleSerial.println("Sending packet...");
    const std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    gsmPort.send(data);

    ConsoleSerial.println("Looping...");
}
#endif


void test_rockpi_power_controller()
{
    ConsoleSerial.println("[Rockpi Power Controller Test]");
    bool is_rockpi_up = piPowerController.isRockPiUp();
    ConsoleSerial.print("[BEGIN] Rockpi is up? ");
    ConsoleSerial.println(is_rockpi_up ? "true" : "false");


    if (is_rockpi_up)
    {
        ConsoleSerial.println("[UP]: Rockpi is Up -> Shutting down");
        piPowerController.commandShutdown();
    }
    else
    {
        ConsoleSerial.println("[DOWN]: Rockpi is Down -> Starting up");
        piPowerController.commandStartup();
    }
    is_rockpi_up = piPowerController.isRockPiUp();
    ConsoleSerial.print("[END] Rockpi is up? ");
    ConsoleSerial.println(is_rockpi_up ? "true" : "false");
}


void test_sleep_and_wake()
{
    ConsoleSerial.println("Sleeping for 5 seconds...");
    WatchdogUtils::sleepFor(5000); // 5 seconds
    ConsoleSerial.println("Woke up!");
}
#endif


void test_setup()
{
#ifdef PLATFORM_ARDUINO
    ConsoleSerial.begin(9600);
    delay(1000);
    // while (!ConsoleSerial) {
    //     digitalWrite(LED_BUILTIN, HIGH);
    //     delay(100);
    //     digitalWrite(LED_BUILTIN, LOW);
    //     delay(100);
    // }
    ConsoleSerial.println("[arduino] Setup: starting...");
#endif

#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif

#ifdef PLATFORM_ARDUINO

    // Inicializa la comunicación serial a 9600 baudios
    // sercom0.resetUART();
    // sercom0.initUART(UART_INT_CLOCK, SAMPLE_RATE_x16, 9600);
    // sercom0.initFrame(UART_CHAR_SIZE_8_BITS, LSB_FIRST, SERCOM_NO_PARITY, SERCOM_STOP_BIT_1);
    // // sercom0.initPads(UART_TX_PAD_2, SERCOM_RX_PAD_3);
    // sercom0.initPads(UART_TX_PAD_0, SERCOM_RX_PAD_1);
    // sercom0.enableUART();


    // softwareSerialSercom0.begin(9600);
    // delay(3000);
    //
    // /// PinPeripheral after begin()
    // pinPeripheral(PIN_A5, PIO_SERCOM_ALT); // TX
    // pinPeripheral(PIN_A6, PIO_SERCOM_ALT); // RX
    // pinPeripheral(PIN_SPI_MOSI, PIO_SERCOM); // TX
    // pinPeripheral(PIN_SPI_SCK, PIO_SERCOM); // RX
    //
    // softwareSerialSercom0.println("Initialized serialSercom0 at 9600 baud");
    // ConsoleSerial.println("Initialized serialSercom0 at 9600 baud");

#endif

    WatchdogUtils::enable(15000); // 15 seconds

    // Inicializa el administrador de la tarjeta SD
    storageManagerRef.begin();

    rtcControllerRef.init();

    rtcControllerRef.setEpoch(1760441400);

    // Logger initialization and configuration
    Logger::initialize(
        &displayRef,
        &storageManagerRef,
        &rtcControllerRef,
        "log.csv", // MAX 8 chars for 8.3 filenames
        Logger::Mode::Both
    );
    Logger::logInfo("================ Setting up Node =================");


    // Initialize the gps
    // gps->init();


#ifdef PLATFORM_ARDUINO
    // batteryController->init();
    solarXBatteryController.init();


    static LambdaTask solarXBatterySyncTask(15000, [&]()
    {
        solarXBatteryController.sync();
    });

    // static MethodTask<BatteryProtectionPolicy> batteryProtectionPolicyTask(
    //     10000, // This must be less than watchdog timeout (10s)
    //     &batteryProtectionPolicy,
    //     &BatteryProtectionPolicy::enforce
    // );
    // scheduler.addTask(&batteryProtectionPolicyTask);

    scheduler.addTask(&solarXBatterySyncTask);


    static FunctionTask batteryTestTask(
        30000,
        [] { withLedIndicator(test_solar_x_battery_controller); }
    );
    scheduler.addTask(&batteryTestTask);
#endif
}


void test_loop()
{
    executeEvery(5000, []
    {
        withLedIndicator([]
        {
            LOG_FREE_MEMORY("[🧪 TEST LOOP START]");
            scheduler.run();
            LOG_FREE_MEMORY("[🧪 TEST LOOP END]");
        });
    });

    // scheduler.run();

    // executeEvery(15000, []
    // {
    //     withLedIndicator([]
    //     {
    //         test_sleep_and_wake();
    //     });
    // });

    // executeEvery(
    //     15000,
    //     test_rockpi_power_controller
    // );

    // executeEvery(
    //     15000,
    //     test_solar_x_battery_controller
    // );

    // executeEvery(15000, [] {
    //     withLedIndicator([] {
    //         test_solar_x_battery_controller();
    //     });
    // });


    // executeEvery(
    //     15000,
    //     [&]{
    //         Logger::logInfo("Lambda every 15 seconds");
    //     }
    // );
}

#ifdef PLATFORM_ARDUINO

#if defined(PLATFORM_HAS_LORA)
void test_onReceiveWrapper(int packetSize)
{
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler()
{
    // mySerial3.IrqHandler();
}

void test_SERCOM1_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom1.IrqHandler();
}

void test_SERCOM0_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom0.IrqHandler();
}


#endif
