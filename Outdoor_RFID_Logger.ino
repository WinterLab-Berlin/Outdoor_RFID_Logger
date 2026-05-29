#include <SD.h>
#include <RTClib.h>
#include <HardwareSerial.h>
#include <ArduinoLowPower.h>
#include "gps_time.h"
#include "defs.h"
#include "read_conf.h"
#include "rfid.h"
#include "logs.h"
#include "poke_rew.h"
#include "rfid_poke_rew.h"

RTC_PCF8523 rtc;

// SD
const int sd_chip_select = 10;

// main state machine states
enum class State {
  GPS_INIT = 1,
  GET_GPS_TIME,
  OPERATE,
  NIGHT
};
State state;

// sleep states
enum class SleepState {
  NO,
  DELAY,
  SLEEP_OPERATE,
  SLEEP_NIGHT,
};
SleepState sleep_state = SleepState::NO;
DateTime state_time;

enum class WakeupReason {
  Regular,
  Timer,
  RfidReader,
  Poke
};
volatile WakeupReason reason = WakeupReason::Regular;

Config config{};
GpsTime gps{};
RfidReader rfid_reader{};
Logs logs{};
PokeRew poke_rew{};
RfidPokeRew rfid_rew{};

void rfid_led_feedback(DateTime& now_time) {
  static int ledState = false;
  static unsigned long time = 0;
  Rfid* rfid = rfid_reader.CheckRfidData(now_time, logs);
  if (rfid->reading) {
    time = millis();
    if (!ledState) {
      ledState = true;
      digitalWrite(LED_BLUE, ledState);
    }
  }

  if (ledState) {
    unsigned long currentMillis = millis();
    if (millis() - time > 1000) {
      ledState = false;
      digitalWrite(LED_BLUE, ledState);
    }
  }
}

void gps_poke_feedback(DateTime& now_time) {
  static int ledState = false;
  static unsigned long time = 0;

  if(digitalRead(POKE_IN)) {
    time = millis();
    if (!ledState) {
      ledState = true;
      digitalWrite(LED_RED, ledState);
    }
  }

  if (ledState) {
    unsigned long currentMillis = millis();
    if (millis() - time > 1000) {
      ledState = false;
      time = currentMillis;
      digitalWrite(LED_RED, ledState);
    }
  } else {
    unsigned long currentMillis = millis();
    if (millis() - time > 1000) {
      ledState = true;
      time = currentMillis;
      digitalWrite(LED_RED, ledState);
    }
  }
}

void sd_error_state() {
  const long interval = 500;
  unsigned long currentMillis = 0;
  unsigned long previousMillis = 0;
  int ledState = LOW;

  while (true) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(LED_GREEN, ledState);
    }
  }
}

void battery_error_state() {
  const long interval = 500;
  unsigned long currentMillis = 0;
  unsigned long previousMillis = 0;
  int ledState = LOW;

  while (true) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(LED_RED, ledState);
      digitalWrite(LED_BLUE, ledState);
      digitalWrite(LED_GREEN, ledState);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  pinMode(RTC_IRQ_PIN, INPUT_PULLUP);
  pinMode(RFID_EN, OUTPUT);
  digitalWrite(RFID_EN, 0);

  pinMode(REWARD_OUT, OUTPUT);
  pinMode(POKE_IN, INPUT);

  gps.gps_init();

  // init serial port to PC, not usable with LowPower
  Serial.begin(19200);

  // time to program the device, otherwise it could switch into the sleep mode.
  // LED on during this time.
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("start");

  // try to read from the SD card
  digitalWrite(LED_GREEN, 1);
  bool init = SD.begin(sd_chip_select);
  if (!init)  // error
    sd_error_state();
  delay(2000);
  digitalWrite(LED_GREEN, 0);
  logs.SetSdInit(init);

  // defaul values
  config.logger_id = LOGGER_ID;
  config.reward = 100;
  config.pause = 500;
  config.max_rewards = 1;
  config.max_pause = 1000;
  config.run_mode = RunMode::Logger;
  config.night_start_hr = NIGHT_START_HR;
  config.night_start_mm = NIGHT_START_MM;
  config.night_end_hr = NIGHT_END_HR;
  config.night_end_mm = NIGHT_END_MM;
  config.gps_get_time_max = GPS_GET_TIME_MAX;

  // load configuration file
  ConfLoader loader{};
  loader.ReadFromSd(&config);
  //config.run_mode = RunMode::LightRew;
  //config.night_start_hr = 10;
  //config.night_start_mm = 10;
  //config.night_end_hr = 10;
  //config.night_end_mm = 15;

  int batLvl = analogRead(BATTERY_STATE);
  //while (true) {
  //  Serial.print("bat: ");
  //  Serial.print(config.min_bat_lvl);
  //  Serial.print(" : ");
  //  Serial.println(batLvl);
  //  delay(1000);
  //}
  if (batLvl < config.min_bat_lvl)
    battery_error_state();

  // start RTC
  rtc.begin();
  if (!rtc.initialized() || rtc.lostPower()) {
    // RTC is NOT initialized, let's set the time!
    // we only need it for initial device programming and in case of problems with GPS.
    // otherwise GPS will alway delive correct time.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
  // timer configuration is not cleared on an RTC reset due to battery backup!
  rtc.deconfigureAllTimers();
  rtc.enableCountdownTimer(PCF8523_FrequencyMinute, 5);
  LowPower.attachInterruptWakeup(RTC_IRQ_PIN, rtc_countdown_over, FALLING);



  // first state on startup
  state = State::GPS_INIT;
  sleep_state = SleepState::NO;
  state_time = rtc.now();
}

void loop() {
  switch (reason) {
    case WakeupReason::Regular:
      break;
    case WakeupReason::RfidReader:
      state_time = rtc.now();
      reason = WakeupReason::Regular;
      break;
    case WakeupReason::Poke:
      state_time = rtc.now();
      reason = WakeupReason::Regular;
      break;
    case WakeupReason::Timer:
      //state_time = rtc.now();
      reason = WakeupReason::Regular;
      break;
  }

  switch (state) {
    // init GPS module
    case State::GPS_INIT:
      {
        Serial.println("GPS_INIT");
        gps.gps_init();
        delay(2000);
        state = State::GET_GPS_TIME;
        state_time = rtc.now();
        sleep_state = SleepState::NO;
        gps.gps_start();
        rfid_reader.Start();
      }
      break;
    // get GPS time
    case State::GET_GPS_TIME:
      {
        // update gps state machine as quick as possible to get and parse all messages
        gps.gps_state_machine(config);

        // gps time is finally availabe or time is over
        if (gps.GetState() == GpsState::AVAILABLE || gps.GetState() == GpsState::NO_TIME) {
          DateTime now_time = rtc.now();
          DateTime adj_time = rtc.now();
          if (gps.GetState() == GpsState::AVAILABLE) {
            adj_time = gps.GetDateTime();
            // gps time is always 1 sec behind due to PPS signal
            adj_time = adj_time + TimeSpan(1);
            rtc.adjust(adj_time);
          }

          // open the data file on SD card
          logs.OpenLogsFile(rtc.now(), config);

          if (gps.GetState() == GpsState::AVAILABLE) {
            logs.LogTimeUpdate(now_time, adj_time);
          } else {
            logs.LogNoTimeUpdate(now_time);
          }

          gps.gps_stop();
          state_time = rtc.now();
          if (is_night(now_time.hour(), now_time.minute())) {
            state = State::NIGHT;
            sleep_state = SleepState::SLEEP_NIGHT;
            digitalWrite(LED_BLUE, 0);
            digitalWrite(LED_RED, 0);

            if (config.run_mode == RunMode::LightRew) {
              detachInterrupt(POKE_IN);
            }
          } else {
            // switch to OPERATE state
            state = State::OPERATE;
            sleep_state = SleepState::SLEEP_OPERATE;
            digitalWrite(LED_BLUE, 0);
            digitalWrite(LED_RED, 0);
            rfid_reader.Start();
          }
        } else {
          DateTime now_time = rtc.now();
          rfid_led_feedback(now_time);
          gps_poke_feedback(now_time);
        }
      }
      break;
    // read rfid messages and store to SD card
    case State::OPERATE:
      {
        DateTime now_time = rtc.now();

        // is day over?
        if (is_night(now_time.hour(), now_time.minute())) {
          // switch to night time
          state = State::NIGHT;
          sleep_state = SleepState::SLEEP_NIGHT;
          state_time = rtc.now();
          rfid_reader.Stop();
          if (config.run_mode == RunMode::LightRew) {
            detachInterrupt(POKE_IN);
          }

          logs.LogNightStart(now_time);
        } else {
          sleep_state = SleepState::SLEEP_OPERATE;

          if (config.run_mode == RunMode::Logger) {
            Rfid* rfid = rfid_reader.CheckRfidData(now_time, logs);
            if (rfid->reading)
              state_time = rtc.now();
          } else if (config.run_mode == RunMode::RfidRew) {
            if (rfid_rew.CheckAndReward(rfid_reader, config, now_time, logs))
              state_time = now_time;
          } else if (config.run_mode == RunMode::LightRew) {
            Rfid* rfid = rfid_reader.CheckRfidData(now_time, logs);
            if (poke_rew.CheckAndReward(config, now_time, logs) || rfid->reading)
              state_time = now_time;
          }
        }
      }
      break;
    // night state: RFID and GPS are disabled, only RTC iterrupt is on
    case State::NIGHT:
      {
        DateTime now_time = rtc.now();
        // is night over?
        if (!is_night(now_time.hour(), now_time.minute())) {
          // lets get GPS time and correct RTC at the beginning of the day
          state = State::GET_GPS_TIME;
          state_time = rtc.now();
          sleep_state = SleepState::NO;
          gps.gps_start();
          poke_rew.Reset();
          rfid_rew.Reset();

          logs.LogNightOver(now_time);
        } else {
          sleep_state = SleepState::SLEEP_NIGHT;
        }
      }
      break;
  }

  // for testing only
  //if (sleep_state == SleepState::SLEEP_NIGHT || sleep_state == SleepState::SLEEP_OPERATE)
  //  sleep_state = SleepState::DELAY;

  switch (sleep_state) {
    // continue the loop immediately
    case SleepState::NO:
      break;
    // continue the loop after a short delay
    case SleepState::DELAY:
      delay(1);
      break;
    // go to sleep and wait for an interrupt
    case SleepState::SLEEP_NIGHT:
      {
        TimeSpan offset = rtc.now() - state_time;
        if (offset.totalseconds() > 5) {
          //digitalWrite(LED_BUILTIN, HIGH);

          rfid_reader.Stop();
          LowPower.deepSleep();
          Serial.begin(19200);

          //digitalWrite(LED_BUILTIN, LOW);
        }
      }
      break;
    case SleepState::SLEEP_OPERATE:
      {
        TimeSpan offset = rtc.now() - state_time;
        if (offset.totalseconds() > 5) {
          //digitalWrite(LED_BUILTIN, LOW);

          Serial1.end();
          Serial.end();
          if (config.run_mode == RunMode::Logger || config.run_mode == RunMode::RfidRew) {
            pinMode(RFID_RX, INPUT);
            LowPower.attachInterruptWakeup(RFID_RX, rfid_iterrrupt, CHANGE);
          } else if (config.run_mode == RunMode::LightRew) {
            rfid_reader.Stop();
            LowPower.attachInterruptWakeup(POKE_IN, poke_iterrupt, RISING);
          }

          LowPower.deepSleep();
          rfid_reader.Start();
          Serial.begin(19200);

          //digitalWrite(LED_BUILTIN, HIGH);
        }
      }
      break;
    default:
      delay(1);
      break;
  }
}

bool is_night(uint8_t hh, uint8_t mm) {
  uint16_t start = config.night_start_hr * 60 + config.night_start_mm;
  uint16_t end = config.night_end_hr * 60 + config.night_end_mm;
  uint16_t time = hh * 60 + mm;

  if (start < end) {
    if (time >= start && time <= end)
      return true;
    else
      return false;
  } else {
    if (time >= start || time <= end)
      return true;
    else
      return false;
  }
}

// Triggered by the PCF8523 Countdown Timer interrupt at the end of a countdown
// period. Meanwhile, the PCF8523 immediately starts the countdown again.
void rtc_countdown_over() {
  reason = WakeupReason::Timer;
}

// triggered by the RFID_RX pin
void rfid_iterrrupt() {
  reason = WakeupReason::RfidReader;
}

void poke_iterrupt() {
  reason = WakeupReason::Poke;
}