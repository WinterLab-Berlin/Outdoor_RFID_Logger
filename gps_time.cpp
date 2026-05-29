#include "USB/USBAPI.h"
#include "gps_time.h"
#include "defs.h"

// create UART for GPS module
Uart Serial2(&sercom1, GPS_RX, GPS_TX, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler() {
  Serial2.IrqHandler();
}

// top of the second for ublox 6 GPS (default setting) is rising edge of PPS time pulse
bool GpsTime::pps_detect() {
  gps_pps_prev = gps_pps_current;
  gps_pps_current = digitalRead(GPS_PPS);
  return (gps_pps_prev == LOW && gps_pps_current == HIGH);  //returns true if PPS has gone high!
}  // end of pps_detect()

/**
 * receive and parse characters from GPS module.
 * only looks for the GPRMC message to get current time and day.
 */
void GpsTime::process_gps() {
  if (Serial2.available() > 0) {  // check if byte ready to read
    uint8_t in_byte = Serial2.read();
    gps_byte_index++;

    switch (gps_rmc_state) {
      case 0:  // check sentence start: GPRMC
        switch (gps_byte_index) {
          case 1 ... 2:
            break;
          case 3:
            gps_rmc_msg = (in_byte == 'R');
            break;
          case 4:
            gps_rmc_msg = (gps_rmc_msg && in_byte == 'M');
            break;
          case 5:
            gps_rmc_msg = (gps_rmc_msg && in_byte == 'C');
            gps_msg_process_flag = gps_rmc_msg;
            if (gps_rmc_msg)
              gps_rmc_state = 1;
            break;
        }
        break;
      case 1:  // check separator
        if (in_byte == ',') {
          gps_rmc_state = 2;
          gps_byte_index = 0;
        } else {
          gps_rmc_state = 0;
          gps_msg_process_flag = false;
        }
        break;
      case 2:  // time parse state
        switch (gps_byte_index) {
          case 1:  // hour tens
            gps_cur_time.hh = (in_byte - '0') * 10;
            break;
          case 2:  // hour units
            gps_cur_time.hh += (in_byte - '0');
            break;
          case 3:  // min tens
            gps_cur_time.mm = (in_byte - '0') * 10;
            break;
          case 4:  // min units
            gps_cur_time.mm += (in_byte - '0');
            break;
          case 5:  // sec tens
            gps_cur_time.ss = (in_byte - '0') * 10;
            break;
          case 6:  // sec units
            gps_cur_time.ss += (in_byte - '0');
            gps_rmc_state = 3;
            gps_byte_index = 0;
            break;
        }
        break;
      case 3:  // check separator, between time and data status
        if (in_byte == ',') {
          gps_rmc_state = 4;
          gps_byte_index = 0;
        }
        break;
      case 4:  // data status state
        if (gps_byte_index == 1) {
          if (in_byte == 'A') {

          } else {
            gps_date_time_avail = false;
            gps_msg_process_flag = false;
          }
        } else if (in_byte == ',') {
          gps_rmc_state = 5;
          gps_byte_index = 0;
        }
        break;
      case 5:  // latitude state
        if (in_byte == ',') {
          gps_rmc_state = 6;
          gps_byte_index = 0;
        }
        break;
      case 6:  // latitude North/South state
        if (in_byte == ',') {
          gps_rmc_state = 7;
          gps_byte_index = 0;
        }
        break;
      case 7:  // longitude state
        if (in_byte == ',') {
          gps_rmc_state = 8;
          gps_byte_index = 0;
        }
        break;
      case 8:  // longitude East/West state
        if (in_byte == ',') {
          gps_rmc_state = 9;
          gps_byte_index = 0;
        }
        break;
      case 9:  // speed state
        if (in_byte == ',') {
          gps_rmc_state = 10;
          gps_byte_index = 0;
        }
        break;
      case 10:  // true course state
        if (in_byte == ',') {
          gps_rmc_state = 11;
          gps_byte_index = 0;
        }
        break;
      case 11:  // date stamp state
        switch (gps_byte_index) {
          case 1:  // day tens
            gps_cur_time.dd = (in_byte - '0') * 10;
            break;
          case 2:
            gps_cur_time.dd += (in_byte - '0');
            break;
          case 3:  // month tens
            gps_cur_time.mo = (in_byte - '0') * 10;
            break;
          case 4:
            gps_cur_time.mo += (in_byte - '0');
            break;
          case 5:  // year tens
            gps_cur_time.yyyy = 2000 + (in_byte - '0') * 10;
            break;
          case 6:
            gps_cur_time.yyyy += (in_byte - '0');
            gps_date_time_avail = true;
            gps_msg_process_flag = false;
            break;
            ;
        }
        break;
      default:
        gps_msg_process_flag = false;
    }
  }
}

/**
 * call periodically this function until GpsState::AVAILABLE or GpsState::NO_TIME
 * to receive date/time from gps module. call gps_start() before.
 */
void GpsTime::gps_state_machine(const Config& conf) {
  switch (gps_state) {
    case GpsState::GET_TIME:
      if (millis() - gps_time > conf.gps_get_time_max) {
        gps_state = GpsState::NO_TIME;
        gps_time = millis();
        digitalWrite(GPS_ENABLE, 0);
      } else {
        if (gps_msg_process_flag)
          process_gps();
        else if (Serial2.available() && Serial2.read() == '$') {  // new sentence -> start over
          gps_rmc_state = 0;
          gps_byte_index = 0;
          gps_msg_process_flag = true;
        }

        if (gps_date_time_avail) {  // valid date and time received
          gps_state = GpsState::GET_PPS;
          gps_time = millis();
          gps_date_time_avail = false;
        }
      }
      break;
    case GpsState::GET_PPS:
      if (pps_detect()) {
        gps_state = GpsState::AVAILABLE;
        gps_time = millis();
        digitalWrite(GPS_ENABLE, 0);
      } else if (millis() - gps_time > 1000) {
        gps_pps_trials++;
        if (gps_pps_trials > 10) {
          // no PPS for some reason, ignore
          gps_state = GpsState::AVAILABLE;
          gps_time = millis();
          digitalWrite(GPS_ENABLE, 0);
        } else {
          gps_state = GpsState::GET_TIME;
          gps_time = millis();
        }
      }
      break;
    case GpsState::AVAILABLE:
      break;
    case GpsState::NO_TIME:
      break;
    default:
      break;
  }
}

/**
 * init the GPS module and state machine.
 */
void GpsTime::gps_init() {
  pinMode(GPS_PPS, INPUT);
  pinMode(GPS_ENABLE, OUTPUT);
  digitalWrite(GPS_ENABLE, 0);
  gps_msg_process_flag = false;

  gps_state = GpsState::GET_TIME;
  Serial2.begin(9600);
  // Assign GPS_RX pin SERCOM functionality
  pinPeripheral(GPS_RX, PIO_SERCOM);
}

/**
 * enable gps module and reset the state machine.
 * call periodically gps_state_machine() until GpsState::AVAILABLE or GpsState::NO_TIME
 * to receive date/time from gps module.
 * do not forget to configurate GPS properly first manually.
 */
void GpsTime::gps_start() {
  gps_state = GpsState::GET_TIME;
  gps_msg_process_flag = false;
  digitalWrite(GPS_ENABLE, 1);
  gps_pps_trials = 0;
  gps_time = millis();
}

void GpsTime::gps_stop() {
  digitalWrite(GPS_ENABLE, 0);
}