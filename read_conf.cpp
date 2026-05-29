#include "read_conf.h"
#include <SD.h>

void ConfLoader::ReadFromSd(Config* conf) {
  if (SD.exists(conf_file)) {
    File file = SD.open(conf_file, FILE_READ);
    if (file) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 0) {
          if (line[0] == '#') {
            // comment line -> ignore
          } else {
            line.trim();

            bool foundEq = false;
            int startInd = -1;
            for (int i = 0; i < line.length(); i++) {
              if (line[i] == '=') {
                foundEq = true;
              } else if (foundEq) {
                if (line[i] != ' ') {
                  startInd = i;
                  break;
                }
              }
            }
            int nbr = strtol(&line[startInd], NULL, 10);

            if (line.startsWith("id")) {
              conf->logger_id = nbr;
            } else if (line.startsWith("reward")) {
              conf->reward = nbr;
            } else if (line.startsWith("pause")) {
              conf->pause = nbr;
            } else if (line.startsWith("max_rewards")) {
              conf->max_rewards = nbr;
            } else if (line.startsWith("run_as")) {
              conf->run_mode = (RunMode)nbr;
              if (conf->run_mode < RunMode::MIN || conf->run_mode > RunMode::MAX)
                conf->run_mode = RunMode::Logger;
            } else if (line.startsWith("max_pause")) {
              conf->max_pause = nbr;
            } else if (line.startsWith("night_start_hr")) {
              conf->night_start_hr = nbr;
            } else if (line.startsWith("night_start_mm")) {
              conf->night_start_mm = nbr;
            } else if (line.startsWith("night_end_hr")) {
              conf->night_end_hr = nbr;
            } else if (line.startsWith("night_end_mm")) {
              conf->night_end_mm = nbr;
            } else if (line.startsWith("min_bat_lvl")) {
              conf->min_bat_lvl = nbr;
            }
          }
        }
      }
      /*
      Serial.printf("id: %d\r\n", conf->logger_id);
      Serial.printf("reward: %d\r\n", conf->reward);
      Serial.printf("pause: %d\r\n", conf->pause);
      Serial.printf("max_rewards: %d\r\n", conf->max_rewards);
      Serial.printf("run_mode: %d\r\n", (int)conf->run_mode);
      Serial.printf("max_pause: %d\r\n", conf->max_pause);
      Serial.printf("night_start_hr: %d\r\n", conf->night_start_hr);
      Serial.printf("night_start_mm: %d\r\n", conf->night_start_mm);
      Serial.printf("night_end_hr: %d\r\n", conf->night_end_hr);
      Serial.printf("night_end_mm: %d\r\n", conf->night_end_mm);
      */
    }
  } else {
    Serial.println("conf.txt doesn't exist.");
  }
}