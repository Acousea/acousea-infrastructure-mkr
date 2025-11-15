#ifndef ACOUSEA_INFRASTRUCTURE_MKR_DEPENDENCIES_NS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_DEPENDENCIES_NS_HPP

// ======================================================================
//  Inclusiones base del proyecto (como en tu código original)
// ======================================================================
#include <libraries.h>

#include "environment/sharedUtils.hpp"

#if __has_include("environment/credentials.hpp")
#include "environment/credentials.hpp"
#else
#error "No credentials file found! Please provide environment/credentials.hpp. Find an example at environment/credentials.example.hpp"
#endif

#if ENVIRONMENT == ENV_PROD
#define ENV_SELECT(PROD_CODE, TEST_CODE) PROD_CODE
#elif ENVIRONMENT == ENV_TEST
#define ENV_SELECT(PROD_CODE, TEST_CODE) TEST_CODE
#else
#error "Unknown ENVIRONMENT value! Please define ENVIRONMENT as ENV_TEST or ENV_PRODUCTION"
#endif

#if defined(PLATFORM_ARDUINO)
#define PLATFORM_SELECT(ARDUINO_CODE, NATIVE_CODE) ARDUINO_CODE
#elif defined(PLATFORM_NATIVE)
#define PLATFORM_SELECT(ARDUINO_CODE, NATIVE_CODE) NATIVE_CODE
#else
#error "Unknown platform! Please define either PLATFORM_ARDUINO or PLATFORM_NATIVE"
#endif

// ======================================================================
//  ConsoleSerial (idéntico a tu cabecera original)
// ======================================================================
#if defined(PLATFORM_ARDUINO)
#define ConsoleSerial SerialUSB
#elif defined(PLATFORM_NATIVE)
static const NativeConsole consoleNative;
#define ConsoleSerial consoleNative
#else
#error "No valid PLATFORM defined. Please define PLATFORM as PLATFORM_ARDUINO or PLATFORM_NATIVE"
#endif

// ======================================================================
//  Namespaces de dependencias (Hardware, Comm, Storage, Logic, System)
//  - Accesores SIEMPRE devuelven referencias (no punteros).
//  - Inicialización perezosa (static local) segura.
// ======================================================================
namespace Dependencies
{
    // ----------------------------------------------------------
    //  Hardware: UARTs / RTC / Battery / GPS / Display / Power
    // ----------------------------------------------------------
    namespace Hardware
    {
        // ========================= UARTs =========================
        // Devuelve Uart& para cada SERCOM software configurado (Arduino).
        // En Native no existe; si lo necesitas, crea mocks parecidos.
#ifdef PLATFORM_ARDUINO
        inline Uart& uart0()
        {
            static Uart instance(
                &sercom0,
                PIN_A6, // RX
                PIN_A5, // TX
                SERCOM_RX_PAD_3,
                UART_TX_PAD_2);
            return instance;
        }

        inline Uart& uart1()
        {
            static Uart instance(
                &sercom1,
                PIN_SPI_SCK, // RX
                PIN_SPI_MOSI, // TX
                SERCOM_RX_PAD_1,
                UART_TX_PAD_0);
            return instance;
        }

        inline ZeroRTCController& zeroRtcController()
        {
            static ZeroRTCController instance;
            return instance;
        }

#endif // PLATFORM_ARDUINO
        inline MockRTCController& mockRtcController()
        {
            static MockRTCController instance;
            return instance;
        }

        // ========================== RTC =========================
        // En tu código actual usas MockRTCController en ambos.
        inline RTCController& rtc()
        {
            // return zeroRtcController();
            return ENV_SELECT(
                zeroRtcController(), // PROD
                mockRtcController() // TEST
            );
        }

        // ======================== Battery =======================
        // También acceso concreto a SolarX (solo si compilas en Arduino)
#ifdef PLATFORM_ARDUINO
        inline SolarXBatteryController& solarXBatteryController()
        {
            // Instancia concreta (coherente con battery())
            static SolarXBatteryController instance(INA219_ADDRESS + 1, INA219_ADDRESS);
            return instance;
        }
#endif

        inline MockBatteryController& mockBatteryController()
        {
            static MockBatteryController instance;
            return instance;
        }

        // En Arduino: SolarX; en Native: Mock
        inline IBatteryController& battery()
        {
            return ENV_SELECT(
                solarXBatteryController(), // PROD
                mockBatteryController() // TEST
            );
        }

        // ========================== GPS =========================
        // Mantengo los tres (UBloxGNSS, MockGPS, MKRGPS); el acceso genérico es IGPS&.
        inline UBloxGNSS& _gpsUblox()
        {
            static UBloxGNSS instance;
            return instance;
        }

        inline MockGPS& _gpsMock()
        {
            static MockGPS instance(0.0, 0.0, 1.0);
            return instance;
        }

        inline MKRGPS& _gpsMKR()
        {
            static MKRGPS instance;
            return instance;
        }

        // Acceso genérico (elige aquí el “por defecto”)
        inline IGPS& gps()
        {
            return ENV_SELECT(
                _gpsUblox(), // PROD
                _gpsMock() // TEST
            );
        }

        // ======================== Display =======================
#ifdef PLATFORM_ARDUINO
        inline SerialArduinoDisplay& arduinoDisplay()
        {
            static SerialArduinoDisplay instance(&ConsoleSerial);
            return instance;
        }
#endif

#ifdef PLATFORM_NATIVE
        inline NativeConsoleDisplay& nativeDisplay()
        {
            static ConsoleDisplay instance;
            return instance;
        }
#endif

        inline IDisplay& display()
        {
#ifdef PLATFORM_ARDUINO
            return arduinoDisplay();
#else
            return nativeDisplay();
#endif
        }

        // ========================= Power ========================
        // MosfetController / PiController / BatteryProtectionPolicy (Arduino)
#ifdef PLATFORM_ARDUINO
        inline MosfetController& mosfet()
        {
            static MosfetController instance;
            return instance;
        }

        inline PiController& pi()
        {
            static PiController instance;
            return instance;
        }
#endif // PLATFORM_ARDUINO

        // ========================= Storage ======================== [SD (Arduino) / HDD (Native)]
#ifdef PLATFORM_ARDUINO
        inline SDStorageManager& sd()
        {
            static SDStorageManager instance;
            return instance;
        }
#endif

#ifdef PLATFORM_NATIVE
        inline HDDStorageManager& hdd()
        {
            static HDDStorageManager instance;
            return instance;
        }
#endif

        inline StorageManager& storage()
        {
            return PLATFORM_SELECT(
                sd(), // ARDUINO
                hdd() // NATIVE
            );
        }
    } // namespace Hardware
    // ----------------------------------------------------------
    //  Comm: Puertos (Serial/Mock/Native/HTTP/LoRa/Iridium/GSM) + Router
    // ----------------------------------------------------------
    namespace Comm
    {
        inline PacketQueue& packetQueue()
        {
            static PacketQueue instance(
                Hardware::storage(),
                Hardware::rtc()
            );
            return instance;
        }

        // =============== Serial / Mocks / HTTP / Native =========
        inline MockSerialPort& _mockSerial()
        {
            static MockSerialPort instance;
            return instance;
        }

#ifdef PLATFORM_ARDUINO
        inline SerialPort& _realSerial()
        {
            static SerialPort instance(Hardware::uart0(), 9600, packetQueue());
            return instance;
        }
#endif

        inline IPort& serial()
        {
            return ENV_SELECT(
                _realSerial(), // PROD
                _mockSerial() // TEST
            );
        }

#ifdef PLATFORM_NATIVE
        inline NativeSerialPort& nativeSerial()
        {
            static NativeSerialPort instance("/tmp/ttyV0", 9600);
            return instance;
        }

        inline HttpPort& http()
        {
            static HttpPort instance("http://127.0.0.1:8000", "123456789012345");
            return instance;
        }
#endif

        // ========================== LoRa ========================
#ifdef PLATFORM_HAS_LORA
        inline LoraPort& lora()
        {
            static LoraPort instance;
            return instance;
        }
        inline MockLoRaPort& _mockLora()
        {
            static MockLoRaPort instance;
            return instance;
        }
#endif

        // ======================== Iridium =======================
        inline MockIridiumPort& _mockIridium()
        {
            static MockIridiumPort instance;
            return instance;
        }

#ifdef PLATFORM_ARDUINO
        inline IridiumPort& _realIridiun()
        {
            static IridiumPort instance(packetQueue());
            return instance;
        }
#endif

        inline IPort& iridium()
        {
            return ENV_SELECT(
                _realIridiun(), // PROD
                _mockIridium() // TEST
            );
        }

        // =========================== GSM ========================
#ifdef PLATFORM_HAS_GSM
        inline const GsmConfig& gsmConfig()
        {
            // Construimos constexpr-like a partir de macros del credentials
            static const GsmConfig config({
                SECRET_PINNUMBER,
                SECRET_GPRS_APN,
                SECRET_GPRS_LOGIN,
                SECRET_GPRS_PASSWORD,
                AWS_MQTT_CLIENT_ID,
                AWS_MQTT_BROKER,
                8883,
                CLIENT_CERTIFICATE,
                CLIENT_PRIVATE_KEY
            });
            return config;
        }

        inline GsmMQTTPort& gsm()
        {
            static GsmMQTTPort instance(gsmConfig(), packetQueue());
            return instance;
        }
#endif // PLATFORM_HAS_GSM

        // ========================= Router =======================
        // En tu init original:
        //   router_({ &gsmPort_, &mockSerialPort_, &mockIridiumPort_ })
        inline Router& router()
        {
            static const std::vector<IPort::PortType> relayingPortsVector = {
#ifdef PLATFORM_HAS_GSM
                IPort::PortType::GsmMqttPort,
#endif
#ifdef PLATFORM_HAS_LORA
                IPort::PortType::LoraPort,
#endif
                IPort::PortType::SBDPort
            };
            static const std::vector<IPort*> ports = {
                &serial(),
                &iridium(),
#ifdef PLATFORM_HAS_LORA
                &lora(),
#endif
#ifdef PLATFORM_HAS_GSM
                &gsm()
#endif

            };
            static Router instance(ports, relayingPortsVector, Comm::packetQueue());
            return instance;
        }
    } // namespace Comm

    // ----------------------------------------------------------
    //  Logic: ModuleProxy, NodeConfigurationRepository, Routines, Runner
    // ----------------------------------------------------------
    namespace Logic
    {
        // ===================== ModuleProxy ======================
        inline std::unordered_map<ModuleProxy::DeviceAlias, IPort::PortType>& devicePortMap()
        {
            static std::unordered_map<ModuleProxy::DeviceAlias, IPort::PortType> map = {
                {ModuleProxy::DeviceAlias::PIDevice, IPort::PortType::SerialPort},
                {ModuleProxy::DeviceAlias::VR2C, IPort::PortType::SerialPort}
            };
            return map;
        }

        inline ModuleProxy& moduleProxy()
        {
#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED

            static ModuleProxy instance(Comm::router(), devicePortMap());
#else
            static ModuleProxy instance(Comm::router(), devicePortMap(), Hardware::storage(), Hardware::rtc());
#endif
            return instance;
        }


        // =========== Node Configuration Repository ==============
        inline NodeConfigurationRepository& nodeConfigurationRepository()
        {
            static NodeConfigurationRepository instance(Hardware::storage());
            return instance;
        }

        inline ModuleManager& moduleManager()
        {
            static ModuleManager instance(
                Logic::nodeConfigurationRepository(),
                Logic::moduleProxy(),
                Hardware::gps(),
                Hardware::battery(),
                Hardware::rtc()
            );
            return instance;
        }

        // ======================== Routines ======================
        // Mantengo exactamente tus cuatro rutinas e inputs
        inline SetNodeConfigurationRoutine& setNodeConfigurationRoutine()
        {
            static SetNodeConfigurationRoutine instance(Logic::moduleManager());
            return instance;
        }

        inline GetUpdatedNodeConfigurationRoutine& getUpdatedNodeConfigurationRoutine()
        {
            static GetUpdatedNodeConfigurationRoutine instance(Logic::moduleManager());
            return instance;
        }

        inline StatusReportingRoutine& completeStatusReportRoutine()
        {
            static StatusReportingRoutine instance(
                Logic::nodeConfigurationRepository(),
                Logic::moduleManager()
            );
            return instance;
        }

        inline StoreNodeConfigurationRoutine& storeNodeConfigurationRoutine()
        {
            static StoreNodeConfigurationRoutine instance(
                nodeConfigurationRepository(),
                moduleProxy());
            return instance;
        }

        inline LogAndRelayErrorPacketRoutine& relayPacketRoutine()
        {
            static LogAndRelayErrorPacketRoutine instance(Comm::router());
            return instance;
        }

        // ===================== Routines Map =====================
        // Igual que en tu código: std::map<uint8_t, std::map<uint8_t, IRoutine<...>*>>.
        inline std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routinesMap()
        {
            static std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> map = {
                {
                    acousea_CommunicationPacket_command_tag,
                    {
                        {acousea_CommandBody_setConfiguration_tag, &setNodeConfigurationRoutine()},
                        {acousea_CommandBody_requestedConfiguration_tag, &getUpdatedNodeConfigurationRoutine()}
                    }
                },
                {
                    acousea_CommunicationPacket_response_tag,
                    {
                        {acousea_ResponseBody_setConfiguration_tag, &storeNodeConfigurationRoutine()},
                        {acousea_ResponseBody_updatedConfiguration_tag, &storeNodeConfigurationRoutine()}
                    }
                },
                {
                    acousea_CommunicationPacket_report_tag,
                    {{acousea_ReportBody_statusPayload_tag, &completeStatusReportRoutine()}}
                },
                {
                    acousea_CommunicationPacket_error_tag, {
                        {acousea_ErrorBody_errorMessage_tag, &relayPacketRoutine()} // Reenvía errores
                    }
                }
            };
            return map;
        }

        // ================= Node Operation Runner ================
        inline NodeOperationRunner& nodeOperationRunner()
        {
            static NodeOperationRunner instance(
                Comm::router(),
                Hardware::storage(),
                nodeConfigurationRepository(),
                routinesMap());
            return instance;
        }

#ifdef PLATFORM_ARDUINO
        inline BatteryProtectionPolicy& batteryProtectionPolicy()
        {
            static BatteryProtectionPolicy instance(
                Hardware::solarXBatteryController(),
                Hardware::pi());
            return instance;
        }

#endif // PLATFORM_ARDUINO
    } // namespace Logic

    // ----------------------------------------------------------
    //  System: Scheduler
    // ----------------------------------------------------------
    namespace System
    {
        inline TaskScheduler& scheduler()
        {
            static TaskScheduler instance;
            return instance;
        }
    } // namespace System
} // namespace Dependencies

#endif // ACOUSEA_INFRASTRUCTURE_MKR_DEPENDENCIES_NS_HPP
