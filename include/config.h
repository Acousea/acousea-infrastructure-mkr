#ifndef MKR1310_DRIFTER_CONFIG_H
#define MKR1310_DRIFTER_CONFIG_H

#include "libraries.h"


// ----------------------------------------------------------------------------------
// Debugging macros
// ----------------------------------------------------------------------------------
#define _DEBUG_MODE_                   false

#if _DEBUG_MODE_
  #define _DEBUG_THROUGH_SERIAL_       true   // Any of these three options can be changed
  #define _DO_STANDBY_                 true   // but only when _DEBUG_MODE_ is true
  #define _GNSS_INDOORS_               true
  #define _SBD_INDOORS_                true
#else
  #define _DEBUG_THROUGH_SERIAL_       true  // Don't change these macros
  #define _DO_STANDBY_                 true
  #define _GNSS_INDOORS_               false
  #define _SBD_INDOORS_                false
#endif

#if _DEBUG_THROUGH_SERIAL_
  #define debugPort                    SerialUSB
  #define _USING_SERIAL_MONITOR_       true
#endif

typedef enum {
  IGNITION_MODE = 0, 
  LAUNCH_MODE,
  DRIFTING_MODE,
  RECOVERY_MODE
} drifter_mode_t;

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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ------------------------------------------------------------------------------------
// RTC
// ------------------------------------------------------------------------------------
RTCZero rtc;

// ------------------------------------------------------------------------------------
// LoRA
// ------------------------------------------------------------------------------------
#define time_dif(now,before)    ((uint32_t)(now - before))

#define LORA_FREQ             868E6
#define LORA_BANDWIDTH        125E3

#define REMOTE_ADDR           0xA0
#define LOCAL_ADDR            0X01

#define REMOTE_SETUP          0xF0
#define REMOTE_DATA_TYPE      0xF1
#define LOCALIZER_ACK         0xF2
// ----------------------------------------------------------------------
// SD
// ----------------------------------------------------------------------
#define SDCARD_SS_PIN   4

#define NONE            0
#define OUT             1
#define IN              2

#define SBD_CHANNEL     0
#define LORA_CHANNEL    1

volatile bool new_lora_message_available = false;
volatile uint8_t new_lora_message_type = 0x00;
volatile uint16_t lora_incomingMsgId = 0;

char logFilename[14]   = "start.log";
char vemcoFilename[14] = "start.vrc";
char commsFilename[14] = "start.com";

// ------------------------------------------------------------------------------------
// SBD MODEM (9603 pinout, S/N 19834, IMEI: 300434064043170)

// Pin	Label	  Description                                    Color
// 1	   RXD	  Iridium 9603N RX (output from RockBLOCK)       Yellow    USED Serial1RX //Serial3TX D0
// 2	   CTS	  Iridium 9603N CTS (output from RockBLOCK)      Brown
// 3	   RTS	  Iridium 9603N RTS (input to RockBLOCK)         Magenta
// 4	   NetAv	Network Available signal                       Blue      USED
// 5	   RI	    Ring Indicator signal (active low)             Green     USED
// 6	   TXD	  Iridium 9603N TX (input to RockBLOCK)          Orange    USED Serial1TX //Serial3RX D1
// 7	   OnOff	Sleep control (pull to ground to switch off)   Gray      USED
// 8	   5v In	5V power supply (450mA limit)                  Red       USED
// 9	   Li-Ion	3.7V power supply (450mA limit)                White
//10	   GND	  Ground                                         Black     USED

// ----------------------------------------------------------------------------------
// mySerial3
// ----------------------------------------------------------------------------------
// Create the new UART instance assigning it to pin 1 and 0
// Uart mySerial3 (&sercom3, 1, 0, SERCOM_RX_PAD_1, UART_TX_PAD_0); 


#define SBD_SLEEP_PIN         2 // OUTPUT, pull to GND to switch off
#define SBD_RING_PIN          3 // INPUT, driven LOW when new messages are available 
#define SBD_MODEM_BAUDS       19200

#define IridiumSerial         Serial1 //mySerial3

IridiumSBD sbd_modem(IridiumSerial, SBD_SLEEP_PIN, SBD_RING_PIN);
bool sbd_is_on = false;
uint8_t _buffer[340];

#define MAX_TAG_MAPS   10
#define MAX_TAGS       10

typedef struct 
{
  uint8_t num_tag_maps;           // How many maps detected (up to MAX_TAG_MAPS)
  uint16_t tagMap[MAX_TAG_MAPS];  // Tag maps (up to MAX_TAG_MAPS)
  
  uint16_t detectedTags;          // How many tags detected (up to MAX_TAGS)
  uint16_t tagID[MAX_TAGS];       // ID of detected tags (up to MAX_TAGS)
  uint8_t  tagMapIndex[MAX_TAGS]; // Index of map in TagMap
  uint16_t detections[MAX_TAGS];  // No. of detections fot this tag
  uint16_t detectionsTotal;       // Sum of all detections
  bool overflow;                  // True if the no of tags exceeded MAX_TAGS
} tagSummary_t;

typedef struct 
{
  time_t epoch;
  float latitude_deg, longitude_deg;
  uint8_t battery_soc;
  time_t received_epoch;
  
  bool includes_vr2c_data;
  tagSummary_t tagSummary;
} remote_data_t;

typedef struct
{
  time_t epoch;
  float latitude_deg, longitude_deg; 
} local_data_t;

// ----------------------------------------------------------------------
// Use this struct to force a mode transition or for reconfiguring the
// current mode time periods
// ----------------------------------------------------------------------
typedef struct {
  drifter_mode_t mode;   // 0, means "keep current mode". This is useful to change timing
                         // without changing mode
  uint32_t sbd_reporting_period_sec; 
  uint32_t lora_comm_period_sec;
  uint32_t gnss_logging_period_sec;   // This should be the shortest
  
} mode_chg_t;


// ----------------------------------------------------------------------
// Message types (4 bits)
// ----------------------------------------------------------------------
#define PROXY_CMD           0x00  // cmd exchange between comm device and server/proxy
#define PARAM_CMD           0x01  // (to boat) ----> Reply is 0x09 (from boat)
#define PACKED_TELEMETRY    0x02  // (from boat)
#define WPCONF_CMD          0x03  // (to boat) ----> Reply is 0x0B (from boat)
#define BEACON_MSG          0x04  // (from boat)
#define DUE_EMERG           0x05  // (from boat)
#define DUE_BASE_CMD        0x06  // (to boat) ----> Reply is 0x0E (from boat)
#define DUE_MSG             0x07  // (from boat)

#define SUPER_PACKET        0x0F  // (from boat) Only generated by wasp

#define PARAM_REPLY         (PARAM_CMD | 0x08)    // 0x09
#define WPCONF_REPLY        (WPCONF_CMD | 0x08)   // 0x0B
#define DUE_BASE_REPLY      (DUE_BASE_CMD | 0x08) // 0x0E

#define DRIFTER_DATA_MSG    (uint8_t)209
#define DRIFTER_SETMODE_MSG (uint8_t)218

#endif //MKR1310_DRIFTER_CONFIG_H
