#pragma once

#include <stdint.h>
#include <RTClib.h>
#include <Arduino.h>         // required before wiring_private.h
#include "wiring_private.h"  // pinPeripheral() function
#include "read_conf.h"

enum class GpsState { GET_TIME,
                      GET_PPS,
                      NO_TIME,
                      AVAILABLE };

/**
 * date and time from GPS module. always 1 second behind of the
 * PPS signal. 
 */
struct gps_time_t {
  uint8_t hh = 0;     // GPS hour part
  uint8_t mm = 0;     // GPS minutes part
  uint8_t ss = 0;     // GPS seconds part
  uint8_t dd = 0;     // GPS day part
  uint8_t mo = 0;     // GPS month part
  uint16_t yyyy = 0;  // GPS year part
};

class GpsTime {
public:
  // top of the second for ublox 6 GPS (default setting) is rising edge of PPS time pulse
  bool pps_detect();

  /**
    * receive and parse characters from GPS module.
    * only looks for the GPRMC message to get current time and day.
    */
  void process_gps();

  /**
    * call periodically this function until GpsState::AVAILABLE or GpsState::NO_TIME
    * to receive date/time from gps module. call gps_start() before.
    */
  void gps_state_machine(const Config& conf);

  /**
   * init the GPS module and state machine.
   */
  void gps_init();

  /**
   * enable gps module and reset the state machine.
   * call periodically gps_state_machine() until GpsState::AVAILABLE or GpsState::NO_TIME
   * to receive date/time from gps module.
   * do not forget to configurate GPS properly first manually.
   */
  void gps_start();

  void gps_stop();

  GpsState GetState() const { return gps_state; };

  DateTime GetDateTime() const { return DateTime(gps_cur_time.yyyy, gps_cur_time.mo, gps_cur_time.dd, gps_cur_time.hh, gps_cur_time.mm, gps_cur_time.ss); };

private:
  GpsState gps_state;
  unsigned long gps_time;

  uint8_t gps_pps_current = HIGH;
  uint8_t gps_pps_prev = HIGH;
  uint8_t gps_pps_trials = 0;

  gps_time_t gps_cur_time;

  bool gps_date_time_avail = false;   // is GPS date and time available?
  bool gps_msg_process_flag = false;  // is GPS sentence processing?
  bool gps_rmc_msg = false;           // is it the RMC sentence?
  uint8_t gps_byte_index = 0;         // gps sentence byte index
  uint8_t gps_rmc_state = 0;          // RMC sentence processing state
};


