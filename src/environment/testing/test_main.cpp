#include "test_main.h"
#include "wiring_private.h"
// #include "WVariant.h"


void test_solar_x_battery_controller() {
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

    Logger::logInfo(
        "[TEST_SOLARX_BATTERY_CONTROLLER_CC]," +
        std::to_string(batteryVoltageVolts) + "," +
        std::to_string(panelVoltageVolts) + "," +
        std::to_string(batteryCurrentAmps) + "," +
        std::to_string(panelCurrentAmps) + "," +
        std::to_string(netPowerWatts) + "," +
        std::to_string(voltageSOCRnd) + "," +
        std::to_string(coulombSOCRnd) + "," +
        std::to_string(combinedSOCRnd) + "," +
        std::to_string(voltageSOCAcc) + "," +
        std::to_string(coulombSOCAcc) + "," +
        std::to_string(combinedSOCAcc) + "," +
        std::to_string(static_cast<int>(batteryStatus))
    );

    Logger::logFreeMemory("[FREEMEM]");
}


void test_gsm_initialization() {
#ifdef PLATFORM_HAS_GSM
    // ------------------------ Test GSM Connection ------------------------
    // gsmPort.init();
    Logger::logFreeMemory("[MAIN] After GSM init");
    ConsoleSerial.println("Preparing to send packet using GSM...");
    // MyGSMSSLClient myGsmSslClient;
    // myGsmSslClient.updateCerts(GSM_ROOT_CERTS, std::size(GSM_ROOT_CERTS));
    // myGsmSslClient.listCertificates(CertType::All);

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

    // gsmPort.testConnection("www.google.com", 443, "/", true);
    // gsmPort.testConnection("antapagon.com", 443, "/", true);
    // gsmPort.testConnection("test432091-framework22.antapagon.com", 443, "/", true);
    // gsmPort.testConnection("www.antapagon.com", 443, "/", true);


    ConsoleSerial.println("Sent packet using GSM");
    // ---------------------------------------------------------------------
#endif
}

void test_battery_usage() {
    // nodeOperationRunner.init();
    // nodeOperationRunner.run();
    const auto percentage = batteryController->voltageSOC_rounded();
    const auto status = batteryController->status();
    Logger::logInfo(
        "Battery: " + std::to_string(percentage) + "%, Status: " + std::to_string(
            static_cast<int>(status))
    );
}

void test_rockpi_power_controller() {
    ConsoleSerial.println("[Rockpi Power Controller Test]");
    bool is_rockpi_up = piPowerController.isRockPiUp();
    ConsoleSerial.print("[BEGIN] Rockpi is up? ");
    ConsoleSerial.println(is_rockpi_up ? "true" : "false");


    if (is_rockpi_up) {
        ConsoleSerial.println("[UP]: Rockpi is Up -> Shutting down");
        piPowerController.commandShutdown();
    } else {
        ConsoleSerial.println("[DOWN]: Rockpi is Down -> Starting up");
        piPowerController.commandStartup();
    }
    is_rockpi_up = piPowerController.isRockPiUp();
    ConsoleSerial.print("[END] Rockpi is up? ");
    ConsoleSerial.println(is_rockpi_up ? "true" : "false");
}


void test_gsm_sending_packets() {
    // Try to send a packet
    const auto packets = gsmPort.read();
    if (packets.empty()) {
        ConsoleSerial.println("No packets received.");
    } else {
        ConsoleSerial.println("Received packets:");
        for (const auto &packet: packets) {
            const auto hexString = Logger::vectorToHexString(packet);
            ConsoleSerial.println(hexString.c_str());
        }
    }

    ConsoleSerial.println("Sending packet...");
    const std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    gsmPort.send(data);

    ConsoleSerial.println("Looping...");
}

void test_sleep_and_wake() {
    ConsoleSerial.println("Sleeping for 10 seconds...");
    // systemMonitor.sleepFor(10000);
    ConsoleSerial.println("Woke up!");
}

void test_setup() {
#ifdef ARDUINO
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
    ENSURE(storageManager, "storageManager");
    ENSURE(display, "display");
    // ENSURE(gps, "gps");
    // ENSURE(rtcController, "rtcController");
    ENSURE(batteryController, "battery");
    ENSURE(&solarXBatteryController, "solarXBatteryController");
    // ENSURE(serialPort, "serialPort");
#ifdef PLATFORM_HAS_GSM
    // ENSURE(&gsmPort, "gsmPort");
#endif
#ifdef PLATFORM_HAS_LORA
    ENSURE(loraPort, "loraPort");
#endif
    // ENSURE(iridiumPort, "iridiumPort");

#ifdef ARDUINO
    ConsoleSerial.println("[arduino] Setup: Ensured pointers");
#endif

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

    // Inicializa el administrador de la tarjeta SD
    storageManager->begin();

    rtcController->init();

    rtcController->setEpoch(1760441400);

    // Logger initialization and configuration
    Logger::initialize(
        display,
        storageManager,
        rtcController,
        "log.csv", // MAX 8 chars for 8.3 filenames
        Logger::Mode::Both
    );
    Logger::logInfo("================ Setting up Node =================");

    // batteryController->init();
    solarXBatteryController.init();

    systemMonitor.init(15000); // 15 seconds

    // Initialize the gps
    // gps->init();

    static MethodTask<SolarXBatteryController> solarXBatterySyncTask(15000,
                                                                     &solarXBatteryController,
                                                                     &SolarXBatteryController::sync);

    static FunctionTask watchDogTask(5000, &SystemMonitor::reset);

    static MethodTask<SystemMonitor> systemMonitorTask(10000, // This must be less than watchdog timeout (10s)
                                                  &systemMonitor,
                                                  &SystemMonitor::protectBattery);
    static FunctionTask batteryTestTask(
        30000,
        [] { withLedIndicator(test_solar_x_battery_controller); }
    );
    scheduler.addTask(&watchDogTask);
    scheduler.addTask(&systemMonitorTask);
    scheduler.addTask(&solarXBatterySyncTask);
    scheduler.addTask(&batteryTestTask);

}


void test_loop() {
    scheduler.run();

    // executeEvery(15000, [] {
        // withLedIndicator([] {
            // test_sleep_and_wake();
        // });
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

#ifdef ARDUINO

#if defined(PLATFORM_HAS_LORA)
void test_onReceiveWrapper(int packetSize) {
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}
#endif

// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void test_SERCOM3_Handler() {
    // mySerial3.IrqHandler();
}

void test_SERCOM1_Handler() {
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom1.IrqHandler();
}

void test_SERCOM0_Handler() {
    // SerialUSB.println("INTERRUPT SERCOM0");
    softwareSerialSercom0.IrqHandler();
}


#endif
