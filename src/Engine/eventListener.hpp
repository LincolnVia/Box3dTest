#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// Each target ID is a radio channel. A receiver gets only changes on its channel.
using ChangeCallBack = void (*)(void *context, size_t id, bool new_state);

struct EventSubscription {
  size_t channel = 0;
  uint64_t token = 0;

  explicit operator bool() const { return token != 0; }
};

class EventListener {
public:
  // Preserve the original listener's 64 usable IDs until map size is known.
  EventListener() : states(64, false), channels(64) {}

  // Call during map loading. IDs in [0, total_ids) become valid.
  void resize_for_map(size_t total_ids) {
    states.resize(total_ids, false);
    channels.resize(total_ids);
  }

  EventSubscription subscribe(size_t id, ChangeCallBack callback,
                              void *context) {
    if (id >= channels.size() || callback == nullptr) {
      return {};
    }

    const uint64_t token = next_token++;
    channels[id].push_back({token, callback, context});
    return {id, token};
  }

  bool unsubscribe(EventSubscription subscription) {
    if (!subscription || subscription.channel >= channels.size()) {
      return false;
    }

    auto &receivers = channels[subscription.channel];
    for (size_t i = 0; i < receivers.size(); ++i) {
      if (receivers[i].token == subscription.token) {
        receivers.erase(receivers.begin() + static_cast<std::ptrdiff_t>(i));
        return true;
      }
    }
    return false;
  }

  bool getState(size_t id) const {
    return id < states.size() ? states[id] : false;
  }

  // Returns false for an out-of-range ID; true when the ID was valid.
  bool setState(size_t id, bool new_state) {
    if (id >= states.size()) {
      return false;
    }
    if (states[id] == new_state) {
      return true;
    }

    states[id] = new_state;

    // Snapshot permits callbacks to subscribe or unsubscribe during dispatch.
    const auto receivers = channels[id];
    for (const auto &receiver : receivers) {
      if (receiver.callback != nullptr) {
        receiver.callback(receiver.context, id, new_state);
      }
    }
    return true;
  }

private:
  struct Receiver {
    uint64_t token;
    ChangeCallBack callback;
    void *context;
  };

  std::vector<bool> states;
  std::vector<std::vector<Receiver>> channels;
  uint64_t next_token = 1;
};
