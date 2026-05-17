#include "TVController.h"
#include <algorithm>
#include <iostream>

TVController::TVController(Tuner *tuner) : tuner(tuner), processingCH("") {}

std::vector<int> TVController::getFavorites() const { return favorites; }

void TVController::setTunerCh() {
  std::cout << "현재 설정하는 채널 : " << processingCH << std::endl;
  tuner->setCH(processingCH);
}

void TVController::pushButton(remoteKey key) {
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
    processingCH += to_string(key);

    if (processingCH.length() == 2) {
      int chNum = std::stoi(processingCH);
      processingCH = std::to_string(chNum);
      setTunerCh();
      processingCH = "";
    }
    break;

  case remoteKey::KEY_OK:
    if (!processingCH.empty()) {
      setTunerCh();
      processingCH = "";
    }
    break;

  case remoteKey::KEY_MENU:
    processingCH = "";
    break;

  case remoteKey::KEY_FAV: {
    int curr = std::stoi(tuner->getCurrentCH());
    auto it = std::find(favorites.begin(), favorites.end(), curr);

    if (it != favorites.end()) {
      favorites.erase(it);
    } else {
      favorites.push_back(curr);
      std::sort(favorites.begin(), favorites.end());
    }
    break;
  }

  case remoteKey::KEY_NEXT_FAV: {
    if (favorites.empty())
      break;

    int curr = std::stoi(tuner->getCurrentCH());
    auto it = std::upper_bound(favorites.begin(), favorites.end(), curr);

    if (it != favorites.end()) {
      tuner->setCH(std::to_string(*it));
    } else {
      tuner->setCH(std::to_string(favorites[0]));
    }
    break;
  }
  }
}