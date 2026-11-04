#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Each target name is a channel. Receivers get changes for their named channel.
using ChangeCallBack = void (*)(void *context, const std::string &target,
                                bool new_state);

struct EventSubscription {
  std::string target;
  uint64_t token = 0;

  explicit operator bool() const { return token != 0; }
};

class EventListener {
public:
  EventSubscription subscribe(const std::string &target,
                              ChangeCallBack callback, void *context) {
    if (target.empty() || callback == nullptr) {
      return {};
    }

    const uint64_t token = next_token++;
    channels[target].push_back({token, callback, context});
    return {target, token};
  }

  bool unsubscribe(EventSubscription subscription) {
    if (!subscription) {
      return false;
    }

    auto channel = channels.find(subscription.target);
    if (channel == channels.end()) {
      return false;
    }
    auto &receivers = channel->second;
    for (size_t i = 0; i < receivers.size(); ++i) {
      if (receivers[i].token == subscription.token) {
        receivers.erase(receivers.begin() + static_cast<std::ptrdiff_t>(i));
        if (receivers.empty()) {
          channels.erase(channel);
        }
        return true;
      }
    }
    return false;
  }

  bool getState(const std::string &target) const {
    const auto state = states.find(target);
    return state != states.end() ? state->second : false;
  }

  // Returns false for an empty target; otherwise true when the state is set.
  bool setState(const std::string &target, bool new_state) {
    if (target.empty()) {
      return false;
    }
    if (getState(target) == new_state && states.contains(target)) {
      return true;
    }

    states[target] = new_state;

    // Snapshot permits callbacks to subscribe or unsubscribe during dispatch.
    const auto channel = channels.find(target);
    if (channel != channels.end()) {
      const auto receivers = channel->second;
      for (const auto &receiver : receivers) {
        if (receiver.callback != nullptr) {
          receiver.callback(receiver.context, target, new_state);
        }
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

  std::unordered_map<std::string, bool> states;
  std::unordered_map<std::string, std::vector<Receiver>> channels;
  uint64_t next_token = 1;
};
