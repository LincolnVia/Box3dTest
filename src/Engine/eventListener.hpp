#pragma once

// I need a event listener
#include <bitset>
#include <cstdint>
class EventListener {

public:
  int getState(int id) { return states[id]; }

  void setState(int id, int state) { states[id] = state; }

private:
  // Stored 0000100001000010001100 each 1 being status holder for each
  // trigger/recieverID status
  std::bitset<64> states;
};
