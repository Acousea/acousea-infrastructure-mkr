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

// IMPORTANT: periods must be multiples of 60 seconds and no shorter than 60 secs
#define SBD_REPORTING_LAUNCH_SEC    300    //   600
#define SBD_REPORTING_DRIFTING_SEC  900    //   3600
#define SBD_REPORTING_RECOVERY_SEC  120    //   300

uint32_t sbd_reporting_period_sec = SBD_REPORTING_LAUNCH_SEC;
uint32_t vr2c_sampling_period_sec = 600;
uint32_t lora_comm_period_sec     = 180;     // 120;
uint32_t gnss_logging_period_sec  = 180;     // 300;

time_t last_sbd_reporting_epoch = 0;
time_t last_lora_comm_epoch = 0;
time_t last_vr2c_sampling_epoch = 0;

uint16_t lora_msg_sent = 0;

typedef enum {
  IGNITION_MODE = 0, 
  LAUNCH_MODE,
  DRIFTING_MODE,
  RECOVERY_MODE
} drifter_mode_t;


  

#define LAUNCH_NUM_CYCLES         3

// This is the initial mode. It will last until IGNITION_NUM_CYCLES, then 
// automatically it will transit into DRIFTING_MODE
drifter_mode_t drifter_mode = IGNITION_MODE;
uint32_t drifter_num_cycles = 0;
uint32_t drifter_SBD_num_cycles = 0;

// ----------------------------------------------------------------------------------
// mySerial3
// ----------------------------------------------------------------------------------
// Create the new UART instance assigning it to pin 1 and 0
Uart mySerial3 (&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0); 

// ----------------------------------------------------------------------------------
// Adafruit's LC709203F
// ----------------------------------------------------------------------------------
Adafruit_LC709203F lc;

// ------------------------------------------------------------------------------------
// OLED DISPLAY
// ------------------------------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...

// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET     -1 

// See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_ADDRESS 0x3D 
Adafruit_SSD1306 adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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
// // GNSS
// // GPS object is declared in the Arduino_MKRGPS library
// // Pin 7 Is used as the extinct pin
// // ------------------------------------------------------------------------------------
// #define GNSS_MAX_FIX_TIME_MS   900000    // 15 minutes
// #define GNSS_WAIT_TIME_MS      120000    // msecs, 2 minutes
// #define GNSS_WAKEUP_PIN        4

// SFE_UBLOX_GNSS myGNSS;

// // ------------------------------------------------------------------------------------
// // LoRA
// // ------------------------------------------------------------------------------------
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

// // -----------------------------------------------------------------------------------
// // VEMCO green cable  - R1IN   - R1OUT MAX232 - MKR RX
// // VEMCO white cable  - T1OUT  - T1IN MAX232  - MKR TX
// // VEMCO shield cable - NC
// // VEMCO DC+ cable    - 12V
// // VEMCO DC- cable    - GND    - GND MAX232   - MKR GND
// //                               3.3V MAX232  - MKR VIN
// // -----------------------------------------------------------------------------------
// #define USE_VR2C                    true

// #define _DEBUG_VR2C         

// #define VRC2_MINI_BAUDS             9600
// #define vr2c_serial                 Serial1

// //#define SSR_VR2C_MINI_PIN           DIGITAL8

// #define VR2C_STATUS_MSG               51
// #define VR2C_DETECTION_MSG            52

// #define VR2C_MAX_MSG_SZ               200
// #define VR2C_STATUS_REPORT_SEC        300UL

// // The VRC2 disables serial transceivers after 30secs of the last transmission or
// // after a QUIT command
// #define VR2C_SERIAL_RXTX_TIMEOUT_MS   28000UL // It should be 30000
// uint32_t lastTxTimestamp_ms = 0UL;
// uint32_t lastSyncTimestamp_ms = 0UL;

// #define VR2C_SERIAL_MSG_TIMEOUT_MS    400UL
// #define SERIAL_CHAR_TIMEOUT           20UL

// const char vrc2_mini_serial_no[7] = "450315"; // "450270";
// uint8_t vr2c_device_summation = 0;

// typedef struct
// { 
//   uint32_t receiver_serialno;
//   uint16_t seqno;
//   uint8_t year, month, day, hour, minute, second;
//   char letter;
//   uint8_t freq;
//   uint16_t tagMap;
//   uint32_t tagID;
//   uint8_t hasSensors; // This may be 0, 1, 2
//   uint8_t sensorADC[2];
//   uint32_t timestamp_ms;
//   float latitude;
//   float longitude;
// } vemco_detection_t;

// typedef struct
// {
//   uint32_t receiver_serialno;
//   uint16_t seqno;
//   uint8_t year, month, day, hour, minute, second;
//   uint32_t dc;
//   uint32_t pc;
//   float lv_v;
//   float bv_v;
//   float bu_percent;
//   float i_ma;
//   float temperature_c;  
//   float du_percent;
//   float ru_percent; 
//   float tilt_g[3];
// } vemco_sst_t;

// #define MAX_TAG_MAPS   10
// #define MAX_TAGS       10

// typedef struct 
// {
//   uint8_t num_tag_maps;           // How many maps detected (up to MAX_TAG_MAPS)
//   uint16_t tagMap[MAX_TAG_MAPS];  // Tag maps (up to MAX_TAG_MAPS)
  
//   uint16_t detectedTags;          // How many tags detected (up to MAX_TAGS)
//   uint16_t tagID[MAX_TAGS];       // ID of detected tags (up to MAX_TAGS)
//   uint8_t  tagMapIndex[MAX_TAGS]; // Index of map in TagMap
//   uint16_t detections[MAX_TAGS];  // No. of detections fot this tag
//   uint16_t detectionsTotal;       // Sum of all detections
//   bool overflow;                  // True if the no of tags exceeded MAX_TAGS
// } tagSummary_t;

// typedef struct 
// {
//   time_t epoch;
//   float latitude_deg, longitude_deg;
//   uint8_t battery_soc;
  
//   bool includes_vr2c_data;
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
//   drifter_mode_t mode;   // 0, means "keep current mode". This is useful to change timing
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