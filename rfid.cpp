#include "rfid.h"

void RfidReader::Start() {
  if (!running) {
    digitalWrite(RFID_EN, HIGH);
    Serial1.begin(19200, SERIAL_8E1);
    running = true;
  }
}

void RfidReader::StartForce() {
  digitalWrite(RFID_EN, HIGH);
  Serial1.begin(19200, SERIAL_8E1);
  running = true;
}

void RfidReader::Stop() {
  if(running){
    digitalWrite(RFID_EN, LOW);
    running = false;
  }
}

Rfid* RfidReader::CheckRfidData(DateTime now_time, Logs& logs) {
  rfid.reading = false;
  rfid.rfid_available = false;

  while (Serial1.available()) {
    int bt = Serial1.read();
    rfid.reading = true;

    if (buffer_pos >= rfid_buffer_size)
      buffer_pos = 0;

    if (buffer_pos == 0) {
      if (bt == RFID_ID) {  // check the device ID
        rfid_buffer[buffer_pos] = bt;
        buffer_pos++;
      }
    } else if (buffer_pos == 1) {
      if (bt == 0x03) {  // MODBUS read function
        rfid_buffer[buffer_pos] = bt;
        buffer_pos++;
      }
    } else if (buffer_pos == 2) {
      mes_length = bt + 5;
      if (mes_length > rfid_buffer_size) {  // too long
        buffer_pos = 0;
      } else {
        rfid_buffer[buffer_pos] = bt;
        buffer_pos++;
      }
    } else {
      rfid_buffer[buffer_pos] = bt;
      buffer_pos++;
    }

    if (buffer_pos == mes_length) {
      uint16_t crc_cal = CRC16_MB(rfid_buffer, mes_length - 2);
      uint16_t crc_mes = (rfid_buffer[mes_length - 1] << 8) | rfid_buffer[mes_length - 2];

      if (crc_cal == crc_mes) {
        logs.LogRfid(now_time, &rfid_buffer[3]);

        rfid.rfid_available = true;
        rfid.rfid_buffer[0] = rfid_buffer[3];
        rfid.rfid_buffer[1] = rfid_buffer[4];
        rfid.rfid_buffer[2] = rfid_buffer[5];
        rfid.rfid_buffer[3] = rfid_buffer[6];
        rfid.rfid_buffer[4] = rfid_buffer[7];
        rfid.rfid_buffer[5] = rfid_buffer[8];
        rfid.rfid_buffer[6] = rfid_buffer[9];
      }
      buffer_pos = 0;
    }
  }

  return &rfid;
}

// CRC for RFID reader messages
uint16_t RfidReader::CRC16_MB(uint8_t* cBuffer, int length) {
  int i, j;
  uint16_t wPolynom = 0xa001;
  uint16_t wCrc = 0xffff;
  for (i = 0; i < length; i++) {
    wCrc ^= cBuffer[i];
    for (j = 0; j < 8; j++) {
      if ((wCrc & 0x0001) > 0) {
        wCrc = (wCrc >> 1) ^ wPolynom;
      } else {
        wCrc = wCrc >> 1;
      }
    }
  }
  return wCrc;
}