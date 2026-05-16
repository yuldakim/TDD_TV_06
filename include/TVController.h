/**
 * Copyright 2020 by Samsung Electronics, Inc.,
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung.
 */

#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "Tuner.h"
#include "remoteKey.h"
#include <algorithm> // 추가됨
#include <iostream>
#include <string>
#include <vector> // 추가됨

class TVController {
private:
  Tuner *tuner;
  std::string processingCH;

  std::vector<int> favorites;

  void setTunerCh() {
    // 로그는 테스트의 결과가 절대 아닙니다. 로그가 있는 것을 테스트로 간주하지
    // 마시기 바랍니다.
    std::cout << "현재 설정하는 채널 : " << processingCH << std::endl;
    tuner->setCH(processingCH);
  }

public:
  explicit TVController(Tuner *tuner) : tuner(tuner), processingCH("") {}
  std::vector<int> getFavorites() const { return favorites; }

  void pushButton(remoteKey key) {
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
        processingCH = ""; // 다음 34를 받기 위해 비워줌
      }
      break;
    case remoteKey::KEY_OK:
      if (!processingCH.empty()) {
        setTunerCh();
        processingCH = "";
      }
      break;
    case remoteKey::KEY_MENU: // S1-4 대응: 다른 버튼이 눌리면
      processingCH = "";      // 입력 중이던 번호를 무효화(삭제)함
      break;
    case remoteKey::KEY_FAV: {
      int curr = std::stoi(tuner->getCurrentCH());

      // 1. 목록에서 현재 채널이 있는지 찾기
      auto it = std::find(favorites.begin(), favorites.end(), curr);

      if (it != favorites.end()) {
        // 2. 이미 있다면 삭제 (S2-2)
        favorites.erase(it);
      } else {
        // 3. 없다면 추가 (S2-1)
        favorites.push_back(curr);
        std::sort(favorites.begin(), favorites.end()); // 명세: 항상 정렬 유지
      }
      break;
    }
    case remoteKey::KEY_NEXT_FAV: {
      // 목록이 비어있으면 아무것도 안 함 (나중 명세 예방책으로 살짝 둠)
      if (favorites.empty())
        break;

      // 1. 현재 채널 가져오기
      int curr = std::stoi(tuner->getCurrentCH());

      // 2. upper_bound로 현재 채널보다 큰 첫 번째 채널 찾기
      auto it = std::upper_bound(favorites.begin(), favorites.end(), curr);

      // 3. 찾았다면 그 채널로 변경! (S3-1 달성)
      if (it != favorites.end()) {
        tuner->setCH(std::to_string(*it));
      }
      break;
    }
    }
  }
};

#endif // TV_CONTROLLER_H
