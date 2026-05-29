#include "rfid_poke_rew.h"
#include "defs.h"
#include "wiring_constants.h"
#include "delay.h"
#include "wiring_digital.h"

RfidPokeRew::RfidPokeRew()
  : poke_time{ 0 }
  , rew_time{ 0 }
  , poke_state{ false }
  , rew_state{ false }
  , rewards{ 0 } 
  , rfid_available{ 0 }
  , rfid_time{ 0 } {};

bool RfidPokeRew::CheckAndReward(RfidReader& reader, const Config& config, DateTime now, Logs& logs) {
  Rfid* rfid = reader.CheckRfidData(now, logs);
  if(rfid->rfid_available) {
    if(rfid_available && !CompareRfid(rfid)) {
      Reset();
    }
    rfid_available = true;
    rfid_time = millis();
    for (int i = 0; i < 7; i++) {
      rfid_buffer[i] = rfid->rfid_buffer[i];
    }
  }

  if(rfid_available) {
    poke_state = digitalRead(POKE_IN);
    if (poke_state) {
      poke_time = millis();
    }

    if (rew_state) {
      int rew = millis() - rew_time;
      if (rew >= config.reward) {
        digitalWrite(REWARD_OUT, LOW);
        rew_state = false;
        rew_time = millis();
      }
    } else if (poke_state) {
      int offset = millis() - rew_time;
      if (offset >= config.pause && rewards < config.max_rewards) {
        digitalWrite(REWARD_OUT, HIGH);
        rew_state = true;
        rew_time = millis();
        rewards++;
        logs.LogRfidReward(now, &rfid_buffer[0], config.reward, rewards, config.max_rewards);
      }
    }
  }

  if (poke_state || rew_state || rfid->reading)
    return true;

  int visit = millis() - poke_time;
  if (visit < config.max_pause)
    return true;

  visit = millis() - rfid_time;
  if (visit < config.max_pause)
    return true;

  // if we return false, then the visit is over and reward is available
  Reset();
  return false;
}

void RfidPokeRew::Reset() {
  poke_state = false;
  rew_state = false;
  rfid_available = false;
  rew_time = 0;
  poke_time = 0;
  rewards = 0;
  rfid_time = 0;
  digitalWrite(REWARD_OUT, LOW);
}

bool RfidPokeRew::CompareRfid(Rfid* rfid) {
  for (int i = 0; i < 7; i++) {
    if(rfid_buffer[i] != rfid->rfid_buffer[i])
      return false;
  }

  return true;
}