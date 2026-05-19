#ifndef FAVORITE_CHANNELS_H
#define FAVORITE_CHANNELS_H

#include <optional>
#include <vector>

class FavoriteChannels {
public:
  void toggle(int channel);
  std::optional<int> nextAfter(int current) const;
  bool empty() const;
  std::vector<int> values() const;

private:
  std::vector<int> channels_;
};

#endif // FAVORITE_CHANNELS_H
