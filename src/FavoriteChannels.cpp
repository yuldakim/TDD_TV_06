#include "FavoriteChannels.h"
#include <algorithm>

void FavoriteChannels::toggle(int channel) {
  auto it = std::find(channels_.begin(), channels_.end(), channel);
  if (it != channels_.end()) {
    channels_.erase(it);
  } else {
    channels_.push_back(channel);
    std::sort(channels_.begin(), channels_.end());
  }
}

std::optional<int> FavoriteChannels::nextAfter(int current) const {
  if (channels_.empty()) {
    return std::nullopt;
  }

  const auto it =
      std::upper_bound(channels_.begin(), channels_.end(), current);
  if (it != channels_.end()) {
    return *it;
  }
  return channels_.front();
}

bool FavoriteChannels::empty() const { return channels_.empty(); }

std::vector<int> FavoriteChannels::values() const { return channels_; }
