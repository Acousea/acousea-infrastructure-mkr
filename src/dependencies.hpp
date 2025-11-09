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
                UART_TX_PAD_2
            );
            return instance;
        }

        inline Uart& uart1()
        {
            static Uart instance(
                &sercom1,
                PIN_SPI_SCK, // RX
                PIN_SPI_MOSI, // TX
                SERCOM_RX_PAD_1,
                UART_TX_PAD_0
            );
            return instance;
        }
#endif // PLATFORM_ARDUINO

        // ========================== RTC =========================
        // En tu código actual usas MockRTCController en ambos.
        inline RTCController& rtc()
        {
            // Si en Arduino quieres ZeroRTCController, cámbialo aquí.
            static MockRTCController instance;
            return instance;
        }

        // ======================== Battery =======================


        // También acceso concreto a SolarX (solo si compilas en Arduino)
        inline SolarXBatteryController& solarXBatteryController()
        {
#ifdef PLATFORM_ARDUINO
            // Instancia concreta (coherente con battery())
            static SolarXBatteryController instance(INA219_ADDRESS + 1, INA219_ADDRESS);
            return instance;
#else
            // Si llamas a esto en Native, no existe SolarX. Forzamos error link-time.
            static_assert(false, "solarXBatteryController() only available on PLATFORM_ARDUINO");
#endif
        }

        // En Arduino: SolarX; en Native: Mock
        inline IBatteryController& battery()
        {
#ifdef PLATFORM_ARDUINO
            return solarXBatteryController();
#else
            static MockBatteryController instance;
            return instance;
#endif
        }

        // ========================== GPS =========================
        // Mantengo los tres (UBloxGNSS, MockGPS, MKRGPS); el acceso genérico es IGPS&.
        inline UBloxGNSS& ublox()
        {
            static UBloxGNSS instance;
            return instance;
        }

        inline MockGPS& mock()
        {
            static MockGPS instance(0.0, 0.0, 1.0);
            return instance;
        }

        inline MKRGPS& mkr()
        {
            static MKRGPS instance;
            return instance;
        }

        // Acceso genérico (elige aquí el “por defecto”)
        inline IGPS& gps()
        {
            // Ahora mismo devuelvo el mock como en tu getter actual
            return mock();
        }

        SerialArduinoDisplay& serialDisplay();

        // ======================== Display =======================
        inline IDisplay& display()
        {
#ifdef PLATFORM_ARDUINO
            static SerialArduinoDisplay instance(&ConsoleSerial);
            return instance;
#else
            static ConsoleDisplay instance;
            return instance;
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

        // ========================= Storage ========================
        // StorageManager: SD (Arduino) / HDD (Native)

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
#ifdef PLATFORM_ARDUINO
            return sd();
#else
            return hdd();
#endif
        }
    } // namespace Hardware
    // ----------------------------------------------------------
    //  Comm: Puertos (Serial/Mock/Native/HTTP/LoRa/Iridium/GSM) + Router
    // ----------------------------------------------------------
    namespace Comm
    {
        // =============== Serial / Mocks / HTTP / Native =========
        inline MockSerialPort& mockSerial()
        {
            static MockSerialPort instance;
            return instance;
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
        inline MockLoRaPort& mockLora()
        {
            static MockLoRaPort instance;
            return instance;
        }
#endif

        // ======================== Iridium =======================
        inline IridiumPort& iridium()
        {
            static IridiumPort instance;
            return instance;
        }

        inline MockIridiumPort& mockIridium()
        {
            static MockIridiumPort instance;
            return instance;
        }

        // =========================== GSM ========================
#ifdef PLATFORM_HAS_GSM
        struct GsmCfgHolder final
        {
            // Config inmutable en flash
            const GsmConfig cfg;

            constexpr GsmCfgHolder(const GsmConfig& c) : cfg(c)
            {
            }
        };

        inline const GsmConfig& gsmConfig()
        {
            // Construimos constexpr-like a partir de macros del credentials
            static const GsmCfgHolder holder({
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
            return holder.cfg;
        }

        inline GsmMQTTPort& gsm()
        {
            static GsmMQTTPort instance(gsmConfig());
            return instance;
        }
#endif // PLATFORM_HAS_GSM

        // ========================= Router =======================
        // En tu init original:
        //   router_({ &gsmPort_, &mockSerialPort_, &mockIridiumPort_ })
        inline Router& router()
        {
            // Construye con los puertos disponibles en esta build.
            // Nota: El Router de tu lib acepta initializer_list<IPort*>.
            // Los accesores públicos siguen dando referencias, pero aquí
            // pasamos punteros internamente por compatibilidad con tu lib.
            static Router instance({
#ifdef PLATFORM_HAS_GSM
                &gsm(),
#endif
                &mockSerial(),
                &mockIridium()
            });
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
            static ModuleProxy instance(Comm::router(), devicePortMap());
            return instance;
        }

        // =========== Node Configuration Repository ==============
        inline NodeConfigurationRepository& nodeConfigurationRepository()
        {
            static NodeConfigurationRepository instance(Hardware::storage());
            return instance;
        }

        // ======================== Routines ======================
        // Mantengo exactamente tus cuatro rutinas e inputs
        inline SetNodeConfigurationRoutine& setNodeConfigurationRoutine()
        {
            static SetNodeConfigurationRoutine instance(
                nodeConfigurationRepository(),
                moduleProxy()
            );
            return instance;
        }

        inline GetUpdatedNodeConfigurationRoutine& getUpdatedNodeConfigurationRoutine()
        {
            static GetUpdatedNodeConfigurationRoutine instance(
                nodeConfigurationRepository(),
                moduleProxy(),
                Hardware::ublox(),
                static_cast<SolarXBatteryController&>(Hardware::battery()),
                Hardware::rtc()
            );
            return instance;
        }

        inline CompleteStatusReportRoutine& completeStatusReportRoutine()
        {
            static CompleteStatusReportRoutine instance(
                nodeConfigurationRepository(),
                moduleProxy(),
                Hardware::ublox(),
                static_cast<SolarXBatteryController&>(Hardware::battery()),
                Hardware::rtc()
            );
            return instance;
        }

        inline StoreNodeConfigurationRoutine& storeNodeConfigurationRoutine()
        {
            static StoreNodeConfigurationRoutine instance(
                nodeConfigurationRepository(),
                moduleProxy()
            );
            return instance;
        }

        inline RelayPacketRoutine& relayPacketRoutine()
        {
            static RelayPacketRoutine instance(
                Comm::router(), {IPort::PortType::GsmMqttPort, IPort::PortType::SBDPort}
            );
            return instance;
        }

        // ===================== Routines Map =====================
        // Igual que en tu código: std::map<uint8_t, std::map<uint8_t, IRoutine<...>*>>.
        inline std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routinesMap()
        {
            static std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> map = {
                {
                    acousea_CommunicationPacket_command_tag, {
                        {acousea_CommandBody_setConfiguration_tag, &setNodeConfigurationRoutine()},
                        {acousea_CommandBody_requestedConfiguration_tag, &getUpdatedNodeConfigurationRoutine()}
                    }
                },
                {
                    acousea_CommunicationPacket_response_tag, {
                        {acousea_ResponseBody_setConfiguration_tag, &storeNodeConfigurationRoutine()},
                        {acousea_ResponseBody_updatedConfiguration_tag, &storeNodeConfigurationRoutine()}
                    }
                },
                {
                    acousea_CommunicationPacket_report_tag, {
                        {acousea_ReportBody_statusPayload_tag, &completeStatusReportRoutine()}
                    }
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
                nodeConfigurationRepository(),
                routinesMap()
            );
            return instance;
        }

        inline BatteryProtectionPolicy& batteryProtectionPolicy()
        {
            static BatteryProtectionPolicy instance(
                Hardware::solarXBatteryController(),
                Hardware::pi()
            );
            return instance;
        }
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


    // ----------------------------------------------------------
    //  Fachada opcional (para mantener sintaxis tipo Dependencies::xxx())
    //  Todos los accesores públicos devuelven REFERENCIAS.
    // ----------------------------------------------------------
    struct Facade final
    {
        // ===== Hardware =====
#ifdef PLATFORM_ARDUINO
        static inline Uart& uart0() { return Hardware::uart0(); }
        static inline Uart& uart1() { return Hardware::uart1(); }
#endif

        static inline RTCController& rtc() { return Hardware::rtc(); }
        static inline IBatteryController& battery() { return Hardware::battery(); }
        static inline IDisplay& display() { return Hardware::display(); }
        static inline IGPS& gps() { return Hardware::gps(); }

#ifdef PLATFORM_ARDUINO
        static inline PiController& pi() { return Hardware::pi(); }
        static inline BatteryProtectionPolicy& batteryProtectionPolicy() { return Logic::batteryProtectionPolicy(); }
        static inline SolarXBatteryController& solarXBatteryController() { return Hardware::solarXBatteryController(); }
#endif

        // ===== Storage =====
        static inline StorageManager& storage() { return Hardware::storage(); }

        // ===== Comm =====
#ifdef PLATFORM_HAS_GSM
        static inline GsmMQTTPort& gsm() { return Comm::gsm(); }
#endif

#ifdef PLATFORM_HAS_LORA
        static inline LoraPort& lora() { return Comm::lora(); }
#endif

        static inline Router& router() { return Comm::router(); }

        // ===== Logic =====
        static inline ModuleProxy& moduleProxy() { return Logic::moduleProxy(); }

        static inline NodeConfigurationRepository& nodeConfigurationRepository()
        {
            return Logic::nodeConfigurationRepository();
        }

        static inline SetNodeConfigurationRoutine& setNodeConfigurationRoutine()
        {
            return Logic::setNodeConfigurationRoutine();
        }

        static inline GetUpdatedNodeConfigurationRoutine& getUpdatedNodeConfigurationRoutine()
        {
            return Logic::getUpdatedNodeConfigurationRoutine();
        }

        static inline CompleteStatusReportRoutine& completeStatusReportRoutine()
        {
            return Logic::completeStatusReportRoutine();
        }

        static inline StoreNodeConfigurationRoutine& storeNodeConfigurationRoutine()
        {
            return Logic::storeNodeConfigurationRoutine();
        }

        static inline std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routinesMap()
        {
            return Logic::routinesMap();
        }

        static inline NodeOperationRunner& nodeOperationRunner() { return Logic::nodeOperationRunner(); }

        // ===== System =====
        static inline TaskScheduler& scheduler() { return System::scheduler(); }
    };
} // namespace Dependencies

// ======================================================================
//  Alias corto: si quieres mantener exactamente "Dependencies::algo()"
//  cambia tus llamadas a "Dependencies::Facade::algo()" o define:
//
//  using DependenciesNS = Dependencies::Facade;
//  ... y llama DependenciesNS::battery(), etc.
// ======================================================================

#endif // ACOUSEA_INFRASTRUCTURE_MKR_DEPENDENCIES_NS_HPP
