#pragma once

#include <SD.h>
#include <RTClib.h>
#include "read_conf.h"

class Logs {
public:
  void SetSdInit(bool is_init) { is_sd_init = is_init; };

  // open the data file on SD card, if not alread opened
  void OpenLogsFile(DateTime now, const Config& config);

  void LogTimeUpdate(DateTime now_time, DateTime adj_time);

  void LogNoTimeUpdate(DateTime now_time);

  void LogNightStart(DateTime now_time);

  void LogNightOver(DateTime now_time);

  void LogRfid(DateTime now_time, uint8_t* start);

  void LogReward(DateTime now_time, int reward, int rew_nbr, int max_rew);

  void LogRfidReward(DateTime now_time, uint8_t* start, int reward, int rew_nbr, int max_rew);

private:
  bool is_sd_init;
  File data_file;
  char str_format[65];
};