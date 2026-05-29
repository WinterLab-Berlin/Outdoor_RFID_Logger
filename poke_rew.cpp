#include "USB/USBAPI.h"
#include "poke_rew.h"

PokeRew::PokeRew()
  : poke_time{ 0 }, rew_time{ 0 }, poke_state{ false }, rew_state{ false }, rewards{ 0 } {};

/**
 * check the state of the poke sensor and take output
 */
bool PokeRew::CheckAndReward(const Config& config, DateTime now, Logs& logs) {
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
      logs.LogReward(now, config.reward, rewards, config.max_rewards);
    }
  }

  if (poke_state || rew_state)
    return true;

  int visit = millis() - poke_time;
  if (visit < config.max_pause)
    return true;

  // if we return false, then the visit is over and reward is available
  Reset();
  return false;
};

/**
 * reset times and internal state
 */
void PokeRew::Reset() {
  poke_state = false;
  rew_state = false;
  rew_time = 0;
  poke_time = 0;
  rewards = 0;
  digitalWrite(REWARD_OUT, LOW);
};