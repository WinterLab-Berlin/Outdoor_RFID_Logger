#pragma once

#include <RTClib.h>
#include "read_conf.h"
#include "logs.h"
#include "rfid.h"

class RfidPokeRew {
public:
  RfidPokeRew();

  /**
   * check the state of the rfid and poke sensor and take output
   */
  bool CheckAndReward(RfidReader& rfid, const Config& config, DateTime now, Logs& logs);

  /**
   * reset times and internal state
   */
  void Reset();

private:
  bool CompareRfid(Rfid* rfid);

  bool poke_state;
  bool rew_state;
  bool rfid_available;
  int rew_time;
  int poke_time;
  int rewards;
  int rfid_time;

  // contains the rfid
  uint8_t rfid_buffer[7];
};