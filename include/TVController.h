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
#include <iostream>
#include <string>

class TVController {
private:
  Tuner *tuner;
  std::string processingCH;

  void setTunerCh() {
    // 로그는 테스트의 결과가 절대 아닙니다. 로그가 있는 것을 테스트로 간주하지
    // 마시기 바랍니다.
    std::cout << "현재 설정하는 채널 : " << processingCH << std::endl;
    tuner->setCH(processingCH);
  }

public:
  explicit TVController(Tuner *tuner) : tuner(tuner), processingCH("") {}

  void pushButton(remoteKey key) {
    switch (key) {
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
    }
  }
};

#endif // TV_CONTROLLER_H
