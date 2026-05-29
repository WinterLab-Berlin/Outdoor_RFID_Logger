#pragma once

#include "defs.h"
#include "logs.h"

struct Rfid {
  // is true each time a byte is read
  bool reading;
  // is true if rfid is complete
  bool rfid_available;
  // contains the new rfid
  uint8_t rfid_buffer[7];
};

class RfidReader {
public:
  void Start();
  void Stop();

  /**
   * check rfid data on software UART and save them in a file on SD card
   * @return pointer to Rfid struct
   */
  Rfid* CheckRfidData(DateTime now_time, Logs& logs);

  // CRC for RFID reader messages
  uint16_t CRC16_MB(uint8_t* cBuffer, int length);

private:
  // buffer for the RFID
  #define rfid_buffer_size 20
  int buffer_pos = 0;
  int mes_length = 0;
  Rfid rfid;
  uint8_t rfid_buffer[20];
  bool running = false;
};