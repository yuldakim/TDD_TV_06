#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "FavoriteChannels.h"
#include "Tuner.h"
#include "remoteKey.h"
#include <string>
#include <vector>

class TVController {
private:
  Tuner *tuner;
  std::string processingCH;
  FavoriteChannels favoriteChannels;

  static bool isDigitKey(remoteKey key);
  static std::string digitString(remoteKey key);

  void setTunerChannel(const std::string &channel);
  void clearProcessingChannel();
  void appendDigit(remoteKey key);
  void commitProcessingChannel();
  void toggleFavoriteChannel();
  void moveToNextFavoriteChannel();
  int currentChannelNumber() const;
  void dispatchNonDigitKey(remoteKey key);

public:
  explicit TVController(Tuner *tuner);

  std::vector<int> getFavorites() const;
  void pushButton(remoteKey key);
};

#endif // TV_CONTROLLER_H
