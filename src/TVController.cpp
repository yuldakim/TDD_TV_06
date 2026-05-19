#include "TVController.h"
#include <iostream>

namespace {
constexpr std::size_t MaxAutoCommitChannelDigits = 2;
} // namespace

TVController::TVController(Tuner *tuner) : tuner(tuner), processingCH("") {}

std::vector<int> TVController::getFavorites() const {
  return favoriteChannels.values();
}

bool TVController::isDigitKey(remoteKey key) {
  switch (key) {
  case remoteKey::KEY_0:
  case remoteKey::KEY_1:
  case remoteKey::KEY_2:
  case remoteKey::KEY_3:
  case remoteKey::KEY_4:
  case remoteKey::KEY_5:
  case remoteKey::KEY_6:
  case remoteKey::KEY_7:
  case remoteKey::KEY_8:
  case remoteKey::KEY_9:
    return true;
  default:
    return false;
  }
}

std::string TVController::digitString(remoteKey key) {
  switch (key) {
  case remoteKey::KEY_0:
    return "0";
  case remoteKey::KEY_1:
    return "1";
  case remoteKey::KEY_2:
    return "2";
  case remoteKey::KEY_3:
    return "3";
  case remoteKey::KEY_4:
    return "4";
  case remoteKey::KEY_5:
    return "5";
  case remoteKey::KEY_6:
    return "6";
  case remoteKey::KEY_7:
    return "7";
  case remoteKey::KEY_8:
    return "8";
  case remoteKey::KEY_9:
    return "9";
  default:
    return "";
  }
}

void TVController::setTunerChannel(const std::string &channel) {
  std::cout << "현재 설정하는 채널 : " << channel << std::endl;
  tuner->setCH(channel);
}

void TVController::clearProcessingChannel() { processingCH.clear(); }

void TVController::appendDigit(remoteKey key) {
  processingCH += digitString(key);

  if (processingCH.size() == MaxAutoCommitChannelDigits) {
    const int chNum = std::stoi(processingCH);
    setTunerChannel(std::to_string(chNum));
    clearProcessingChannel();
  }
}

void TVController::commitProcessingChannel() {
  if (processingCH.empty()) {
    return;
  }
  setTunerChannel(processingCH);
  clearProcessingChannel();
}

int TVController::currentChannelNumber() const {
  return std::stoi(tuner->getCurrentCH());
}

void TVController::toggleFavoriteChannel() {
  favoriteChannels.toggle(currentChannelNumber());
}

void TVController::moveToNextFavoriteChannel() {
  const auto nextChannel = favoriteChannels.nextAfter(currentChannelNumber());
  if (!nextChannel.has_value()) {
    return;
  }
  tuner->setCH(std::to_string(*nextChannel));
}

void TVController::dispatchNonDigitKey(remoteKey key) {
  switch (key) {
  case remoteKey::KEY_OK:
    commitProcessingChannel();
    break;
  case remoteKey::KEY_MENU:
    clearProcessingChannel();
    break;
  case remoteKey::KEY_FAV:
    toggleFavoriteChannel();
    break;
  case remoteKey::KEY_NEXT_FAV:
    moveToNextFavoriteChannel();
    break;
  default:
    break;
  }
}

void TVController::pushButton(remoteKey key) {
  if (isDigitKey(key)) {
    appendDigit(key);
    return;
  }
  dispatchNonDigitKey(key);
}
