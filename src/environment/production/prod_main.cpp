#include "prod_main.h"

namespace logic = Dependencies::Logic;
namespace hardware = Dependencies::Hardware;
namespace sys = Dependencies::System;
namespace comm = Dependencies::Comm;
namespace shared = SharedUtils;
namespace watchdog = WatchdogUtils;

// ------------------- Main Setup ------------------
void prod_setup()
{
#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: starting...\n");
#endif

#ifdef PLATFORM_ARDUINO
    // Inicializa la comunicación serial a 9600 baudios
    ConsoleSerial.begin(9600);
    // softwareSerialSercom0.begin(9600);
    // softwareSerialSercom1.begin(9600);
    delay(2000); // Espera a que el monitor serie esté listo

    pinPeripheral(PIN_A5, PIO_SERCOM_ALT); // TX
    pinPeripheral(PIN_A6, PIO_SERCOM_ALT); // RX
    pinPeripheral(PIN_SPI_MOSI, PIO_SERCOM); // TX
    pinPeripheral(PIN_SPI_SCK, PIO_SERCOM); // RX

#endif


#if defined(_WIN32) && defined(PLATFORM_NATIVE) && !defined(ARDUINO)
    std::printf("[native] Setup: Began storageManager\n");
#endif

    // Set a custom error handler
    ErrorHandler::setHandler([]()
    {
        ConsoleSerial.println("Custom handler invoked!");
    });

    // Handle an error should trigger the watchdog
    // ErrorHandler::handleError("Failed to initialize module.");

    // Initialize the storage manager
    hardware::storage().begin(); // ALWAYS INITIALIZE STORAGE BEFORE LOGGING FOR THE FIRST TIME

    // Initialize the RTC
    hardware::rtc().init();

    // Initialize the gps
    hardware::gps().init();

    // Logger initialization and configuration
    Logger::initialize(
        &hardware::display(),
        &hardware::storage(),
        &hardware::rtc(),
        "log", // MAX 8 chars for 8.3 filenames
        Logger::Mode::Both
    );

    Logger::logInfo("================ Setting up Node =================");

    // Sync RTC time with GPS time at startup
    hardware::rtc().syncTime(hardware::gps().getTimestamp());

    // ------------ Initialize communication ports ------------
    // Initialize the packet queue
    comm::packetQueue().begin();

    // Initialize the serial communicator
    comm::serial().init();

    // Initialize the LoRa communicator
#ifdef PLATFORM_HAS_LORA
    comm::lora().init();
#endif

    // Initialize the GSM communicator
#ifdef PLATFORM_HAS_GSM
    comm::gsm().init();
#endif
#ifdef PLATFORM_ARDUINO
    // Initialize the Iridium communicator
    comm::iridium().init();
#endif
    // --------------------------------------------------------

    // Initialize the battery protection policy
#ifdef PLATFORM_ARDUINO
    // Initialize the battery controller
    // hardware::battery().init();

    hardware::solarXBatteryController().init();

    // *** SolarX Battery Sync Task *** (Battery policy internally syncs the battery data)
    static MethodTask<BatteryProtectionPolicy> batteryProtectionTask(
        30000, // This must be less than watchdog timeout (10s)
        &logic::batteryProtectionPolicy(),
        &BatteryProtectionPolicy::enforce
    );

    sys::scheduler().addTask(&batteryProtectionTask);
#endif

    // Initialize the node configuration repository
    logic::nodeConfigurationRepository().init();

    // Initialize the Node Operation Runner
    logic::nodeOperationRunner().init();

    // *** Node Operation Runner Task ***
    static MethodTask<NodeOperationRunner> nodeOperationTask(
        15000, // 15 seconds
        &logic::nodeOperationRunner(),
        &NodeOperationRunner::run
    );
    sys::scheduler().addTask(&nodeOperationTask);

#if MODE == DRIFTER_MODE
    // saveDrifterConfig();
#elif MODE == LOCALIZER_MODE
    // saveLocalizerConfig();
#endif

    watchdog::enable(watchdog::DEFAULT_WATCHDOG_TIMEOUT_MS); // 15 seconds
}

// ------------------- Main Loop ------------------
void prod_loop()
{
    constexpr unsigned int LOOP_INTERVAL_MS = WatchdogUtils::DEFAULT_WATCHDOG_TIMEOUT_MS / 2;
    shared::executeEvery(LOOP_INTERVAL_MS, []()
    {
        shared::withLedIndicator([]
        {
            LOG_FREE_MEMORY("[🚀 PROD LOOP START]");
            sys::scheduler().run();
            LOG_FREE_MEMORY("[🚀 PROD LOOP END]");
        });
    });
}

// ------------------ Interrupt Handlers ------------------
#ifdef PLATFORM_ARDUINO

#if defined(PLATFORM_HAS_LORA)
void prod_onReceiveWrapper(int packetSize)
{
    SerialUSB.println("OnReceiveWrapper Callback");
    realLoraPort.onReceive(packetSize);
}
#endif

void prod_SERCOM0_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    hardware::uart0().IrqHandler();
}

void prod_SERCOM1_Handler()
{
    // SerialUSB.println("INTERRUPT SERCOM0");
    hardware::uart1().IrqHandler();
}


// Attach the interrupt handler to the SERCOM (DON'T DELETE Essential for the mySerial3 to work)
void prod_SERCOM3_Handler()
{
    mySerial3.IrqHandler();
}


#endif
