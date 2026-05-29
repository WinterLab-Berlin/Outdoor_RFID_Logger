#include "logs.h"

void Logs::OpenLogsFile(DateTime now, const Config& config) {
  if (is_sd_init && !data_file) {
    sprintf(str_format, "%d%02d%02d.csv", now.year(), now.month(), now.day());
    data_file = SD.open(str_format, FILE_WRITE);

    sprintf(str_format, "id,%d,%d", config.logger_id, (int)config.run_mode);
    data_file.write(str_format);
    data_file.write("\r\n");

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogTimeUpdate(DateTime now_time, DateTime adj_time) {
  if (data_file) {
    sprintf(str_format, "time update,%02d.%02d.%d %02d:%02d:%02d,%02d.%02d.%d %02d:%02d:%02d",
            now_time.day(), now_time.month(), now_time.year(), now_time.hour(), now_time.minute(), now_time.second(),
            adj_time.day(), adj_time.month(), adj_time.year(), adj_time.hour(), adj_time.minute(), adj_time.second());

    data_file.write(str_format);
    data_file.write("\r\n");
    data_file.flush();

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogNoTimeUpdate(DateTime now_time) {
  if (data_file) {
    sprintf(str_format, "no time update,%02d.%02d.%d %02d:%02d:%02d",
            now_time.day(), now_time.month(), now_time.year(), now_time.hour(), now_time.minute(), now_time.second());

    data_file.write(str_format);
    data_file.write("\r\n");
    data_file.flush();

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogNightStart(DateTime now_time) {
  if (data_file) {
    sprintf(str_format, "night start,%02d.%02d.%d %02d:%02d:%02d,",
            now_time.day(), now_time.month(), now_time.year(), now_time.hour(), now_time.minute(), now_time.second());
    if (data_file) {
      data_file.write(str_format);
      data_file.write("\r\n");
      data_file.flush();
    }

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogNightOver(DateTime now_time) {
  if (data_file) {
    sprintf(str_format, "night over,%02d.%02d.%d %02d:%02d:%02d,",
            now_time.day(), now_time.month(), now_time.year(), now_time.hour(), now_time.minute(), now_time.second());
    if (data_file) {
      data_file.write(str_format);
      data_file.write("\r\n");
      data_file.flush();
    }

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogRfid(DateTime now, uint8_t* rfid_start) {
  if (data_file) {
    sprintf(str_format, "rfid,%02d.%02d.%d %02d:%02d:%02d,%02X%02X%02X%02X%02X%02X%02X",
            now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(),
            rfid_start[0], rfid_start[1], rfid_start[2], rfid_start[3], rfid_start[4], rfid_start[5], rfid_start[6]);

    if (data_file) {
      data_file.write(str_format);
      data_file.write("\r\n");
      data_file.flush();
    }

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogReward(DateTime now, int reward, int rew_nbr, int max_rew) {
  if (data_file) {
    sprintf(str_format, "reward,%02d.%02d.%d %02d:%02d:%02d,%d,%d,%d",
            now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(),
            reward, rew_nbr, max_rew);

    if (data_file) {
      data_file.write(str_format);
      data_file.write("\r\n");
      data_file.flush();
    }

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}

void Logs::LogRfidReward(DateTime now, uint8_t* rfid_start, int reward, int rew_nbr, int max_rew) {
  if (data_file) {
    sprintf(str_format, "rfid_reward,%02d.%02d.%d %02d:%02d:%02d,%02X%02X%02X%02X%02X%02X%02X,%d,%d,%d",
            now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(),
            rfid_start[0], rfid_start[1], rfid_start[2], rfid_start[3], rfid_start[4], rfid_start[5], rfid_start[6],
            reward, rew_nbr, max_rew);

    if (data_file) {
      data_file.write(str_format);
      data_file.write("\r\n");
      data_file.flush();
    }

    if (Serial.availableForWrite())
      Serial.println(str_format);
  }
}