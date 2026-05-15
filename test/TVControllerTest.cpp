#include "TVController.h"
#include "Tuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockTunerForController : public Tuner {
public:
  MOCK_METHOD(std::string, seekCH, (), (override));
  MOCK_METHOD(void, setCH, (const std::string &ch), (override));
  MOCK_METHOD(std::string, getCurrentCH, (), (override));
};

TEST(TVControllerTest, Press1AndConfirmShouldChangeToChannel1) {
  MockTunerForController mockTuner;

  EXPECT_CALL(mockTuner, setCH("1")).Times(1);

  TVController controller(&mockTuner);

  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_OK);
}

TEST(TVControllerTest, PressMultipleDigitsAndConfirmShouldChangeChannel) {
  MockTunerForController mockTuner;

  // 1. 기대 정의: '1' 누르고 '2' 누르고 'OK' 누르면 -> setCH("12")가 호출되어야
  // 함!
  EXPECT_CALL(mockTuner, setCH("12")).Times(1);

  TVController controller(&mockTuner);

  // 2. 실행
  controller.pushButton(remoteKey::KEY_1);
  // 아직 KEY_2가 없으니 일단 KEY_1을 한 번 더 눌러 "11"을 만드는 테스트
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_OK);
}