#ifndef CONFIG_H
#define CONFIG_H
// ----------------------------------------------------------------------------------
// Debugging macros
// ----------------------------------------------------------------------------------
#define _DEBUG_MODE_                   true // true

#if _DEBUG_MODE_
  #define _DEBUG_THROUGH_SERIAL_       true   // Any of these three options can be changed
  #define _DO_STANDBY_                 true   // but only when _DEBUG_MODE_ is true
  #define _GNSS_INDOORS_               true  // 
  #define _SBD_INDOORS_                true  //
#else
  #define _DEBUG_THROUGH_SERIAL_       false  // Don't change these macros
  #define _DO_STANDBY_                 true
  #define _GNSS_INDOORS_               false
  #define _SBD_INDOORS_                false
#endif

#if _DEBUG_THROUGH_SERIAL_
  #define debugPort                    SerialUSB
  #define _USING_SERIAL_MONITOR_       true
#endif

// Just for testing 
#warning "Update communication/logging periods"

// Pin definitions
#define SDCARD_SS_PIN  4

// IMPORTANT: periods must be multiples of 60 seconds and no shorter than 60 secs
#define SBD_REPORTING_LAUNCH_SEC    300    //   600
#define SBD_REPORTING_DRIFTING_SEC  900    //   3600
#define SBD_REPORTING_RECOVERY_SEC  120    //   300


uint32_t lora_comm_period_sec     = 180;     // 120;
uint32_t gnss_logging_period_sec  = 180;     // 300;



// ----------------------------------------------------------------------------------
// mySerial3
// ----------------------------------------------------------------------------------
// Create the new UART instance assigning it to pin 1 and 0
// Uart mySerial3 (&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0); 

// ----------------------------------------------------------------------------------
// Adafruit's LC709203F (Li-ION battery fuel gauge)
// ----------------------------------------------------------------------------------
Adafruit_LC709203F lc;

// ---------------------------------------------------------------------------------
// IRIDIUM 9602 SBD MODEM
//
// https://docs.rockblock.rock7.com/docs/network-availability-and-ring-alert-pinouts
//
// PIN
//  1  IRIDIUM 9602 RX (white)  <------> SERIAL3 TX (D0)
//  2  CTS
//  3  RTS
//  4  NETAV
//  5  RI Ring indicator <------------->
//  6  IRIDIUM 9602 TX (green)  <------> SERIAL3 RX (D1)
//
//  Pull to ground to switch off
//  7  IRIDIUM 9602 SLEEP (yellow) <---> D2 
//  8  IRIDIUM 9602 5V (red)    <------> 5V
//  9  Li-ION 3.7V IN
// 10  IRIDIUM 9602 GND (black) <------> GND
//
// ----------------------------------------------------------------------------------
// #define SBD_SLEEP_PIN         2 // OUTPUT, pull to GND to switch off
// #define SBD_RING_PIN          3 // INPUT, driven LOW when new messages are available 
// #define SBD_MODEM_BAUDS   19200

// #define IridiumSerial mySerial3

// IridiumSBD sbd_modem(IridiumSerial, SBD_SLEEP_PIN, SBD_RING_PIN);
// bool sbd_is_on = false;

// // ------------------------------------------------------------------------------------
// // RTC
// // ------------------------------------------------------------------------------------
// RTCZero rtc;
// volatile uint16_t _rtcFlag = 0;


// // ------------------------------------------------------------------------------------
// // LoRA
// // ------------------------------------------------------------------------------------
// Estructura para almacenar la configuración de la radio

// #define time_dif(now,before)    ((uint32_t)(now - before))

// #define LORA_FREQ               868E6
// #define LORA_BANDWIDTH          125E3

// #define REMOTE_ADDR             0xA0
// #define LOCAL_ADDR              0X01

// #define REMOTE_SETUP            0xF0
// #define REMOTE_DATA_TYPE        0xF1
// #define LOCALIZER_ACK           0xF2

// #define LORA_RX_WINDOW_MS       uint32_t(20000)

// volatile bool new_lora_message_available = false;

// // LoRaModem modem;

// typedef struct 
// {
//   time_t epoch;
//   float latitude_deg, longitude_deg;
//   uint8_t battery_soc;
//   tagSummary_t tagSummary;
// } data_info_t;

// // ----------------------------------------------------------------------
// // Types of messages the drifter can receive via LoRa
// // ----------------------------------------------------------------------
// typedef struct {
//   uint16_t msgNo;
//   float latitude;
//   float longitude;
// } lora_ack_t;

// // ----------------------------------------------------------------------
// // Use this struct to force a mode transition or for reconfiguring the
// // current mode time periods
// // ----------------------------------------------------------------------
// typedef struct {
//   OPERATION_MODES mode;   // 0, means "keep current mode". This is useful to change timing
//                          // without changing mode
//   uint32_t sbd_reporting_period_sec; 
//   uint32_t lora_comm_period_sec;
//   uint32_t gnss_logging_period_sec;   // This should be the shortest
  
// } mode_chg_t;

// typedef struct {
//   time_t timestamp_epoch; 
//   uint8_t type;
//   uint8_t payload_sz;
//   union {
//     lora_ack_t ack;
//     mode_chg_t mode;
//   } data;
// } lora_rx_msg_t;


// // ----------------------------------------------------------------------
// // Message types (4 bits)
// // ----------------------------------------------------------------------
// #define PROXY_CMD           0x00  // cmd exchange between comm device and server/proxy
// #define PARAM_CMD           0x01  // (to boat) ----> Reply is 0x09 (from boat)
// #define PACKED_TELEMETRY    0x02  // (from boat)
// #define WPCONF_CMD          0x03  // (to boat) ----> Reply is 0x0B (from boat)
// #define BEACON_MSG          0x04  // (from boat)
// #define DUE_EMERG           0x05  // (from boat)
// #define DUE_BASE_CMD        0x06  // (to boat) ----> Reply is 0x0E (from boat)
// #define DUE_MSG             0x07  // (from boat)

// #define SUPER_PACKET        0x0F  // (from boat) Only generated by wasp

// #define PARAM_REPLY         (PARAM_CMD | 0x08)    // 0x09
// #define WPCONF_REPLY        (WPCONF_CMD | 0x08)   // 0x0B
// #define DUE_BASE_REPLY      (DUE_BASE_CMD | 0x08) // 0x0E

// #define DRIFTER_DATA_MSG    (uint8_t)209
// #define DRIFTER_SETMODE_MSG (uint8_t)218

// // ----------------------------------------------------------------------
// // SD
// // ----------------------------------------------------------------------
// #define SDCARD_SS_PIN   4

// #define NONE           0
// #define OUT            1
// #define IN             2

// #define SBD_CHANNEL    0
// #define LORA_CHANNEL   1

// char logFilename[14] = "start.log";
// char vemcoFilename[14] = "start.vrc";
// char commsFilename[14] = "start.com";

#endif // CONFIG_H