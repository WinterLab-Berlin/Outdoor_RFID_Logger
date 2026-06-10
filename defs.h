#pragma once

#define LOGGER_ID 1        // logger ID, appears in the log file

#define RFID_EN 16         // RFID reader enable pin
#define RFID_RX 0          // RFID reader transmit line is connected to RX on Feather
#define RFID_ID 3          // each RFID device has an ID. we need this ID to validate messages from the device.

#define NIGHT_START_HR 16   // 0-23, UTC
#define NIGHT_START_MM 10   // 0-59
#define NIGHT_END_HR 9     // 0-23, UTC
#define NIGHT_END_MM 50    // 0-59

#define GPS_RX 11          // RX pin on Feather for the TX line of GPS module
/**
 * TX pin on Feather for the RX line of GPS module.
 * Because pin 10 is already used by SD, we cannot send messages to GPS module.
 * But we still need a pin number here. It can be 10 or something else.
 */
#define GPS_TX 10
#define GPS_ENABLE 15      // enable/disable line for GPS module
#define GPS_PPS 14         // to receiving the PPS signal from GPS
/**
 * max time to wait for valid time from GPS module, in milliseconds.
 * this should also be take into account for NIGHT_END_, because it
 * starts right after the NIGHT_END_.
 */
#define GPS_GET_TIME_MAX 5UL * 60 * 1000

#define RTC_IRQ_PIN 5      // pin to receive time interrupts from RTC

#define REWARD_OUT 6       // reward output pin, e.g. valve or pump
#define POKE_IN    11      // poke input pin, light barrier

#define LED_BLUE 17
#define LED_GREEN 18
#define LED_RED 12

#define BATTERY_STATE A7 // analog input for the battery state