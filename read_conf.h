#pragma once

enum class RunMode {
  MIN = 0,
  Logger = 0,
  RfidRew = 1,
  LightRew = 2,
  MAX = LightRew,
};

struct Config {
  // logger id, overwrites the default one
  int logger_id = 1;
  // reward in milliseconds
  int reward = 100;
  // reward the animal again after this pause, if it still there
  int pause = 250;
  // max number of rewards on a visit
  int max_rewards = 1;
  // wait for next rfid or light barrier event
  int max_pause = 1000;
  // running mode: 0 - rfid logger, 1 - reward on rfid and light barrier, 2 - reward on light barrier
  RunMode run_mode = RunMode::Logger;
  // 0-23, UTC
  int night_start_hr = 18;
  // 0-59
  int night_start_mm = 0;
  // 0-23, UTC
  int night_end_hr = 7;
  // 0-59
  int night_end_mm = 55;
  unsigned long gps_get_time_max;
  // mimimal valid battery level
  int min_bat_lvl = 0;
};

class ConfLoader {
public:
  void ReadFromSd(Config* conf);

private:
  const char* conf_file = "conf.txt";
};