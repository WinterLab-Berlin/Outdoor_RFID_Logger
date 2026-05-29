#pragma once

#include <RTClib.h>
#include "wiring_constants.h"
#include "delay.h"
#include "wiring_digital.h"
#include "read_conf.h"
#include "defs.h"
#include "logs.h"

class PokeRew {
public:
  PokeRew();

  /**
   * check the state of the poke sensor and take output
   */
  bool CheckAndReward(const Config& config, DateTime now, Logs& logs);

  /**
   * reset times and internal state
   */
  void Reset();

private:
  bool poke_state;
  bool rew_state;
  int rew_time;
  int poke_time;
  int rewards;
};