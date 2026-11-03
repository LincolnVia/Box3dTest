#pragma once

// I need a event listener
#include <bitset>
#include <cstddef>
#include <functional>
class EventListener {

public:
  void onChange(ChangeCallBack callback) { onCallBack = callback; }
  int getState(size_t id) { return states.test(id); }

  void setState(size_t id, bool new_state) {
    bool old_state = states.test(id);
    states.set(id, new_state);

    if (old_state != new_state && onCallBack) {
      onCallBack(id, new_state);
    }
  }

private:
  // Stored 0000100001000010001100 each 1 being status holder for each
  // trigger/recieverID status
  std::bitset<64> states;
  ChangeCallBack onCallBack = nullptr;
};

/* int main() {
    EventListener listener;



    listener.setState(12, true);  // Triggers callback
    listener.setState(12, true);  // No change, does not trigger
    listener.setState(12, false); // Triggers callback
}
*/
