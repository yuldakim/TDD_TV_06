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
      processingCH += to_string(key);

      if (processingCH.length() == 2) {
        setTunerCh();
        processingCH = ""; // 다음 34를 받기 위해 비워줌
      }
      break;
    case remoteKey::KEY_OK:
      setTunerCh();
      processingCH = "";
      break;
    }
  }
};

#endif // TV_CONTROLLER_H
