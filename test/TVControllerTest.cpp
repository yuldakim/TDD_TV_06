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

  EXPECT_CALL(mockTuner, setCH("12")).Times(1);

  TVController controller(&mockTuner);

  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);
  controller.pushButton(remoteKey::KEY_OK);
}

TEST(TVControllerTest, S1_3_ContinuousInput) {
  MockTunerForController mockTuner;

  // 기대 정의: "12"와 "11"이 각각 한 번씩 호출되면 통과!
  EXPECT_CALL(mockTuner, setCH("12")).Times(1);
  EXPECT_CALL(mockTuner, setCH("34")).Times(1);

  TVController controller(&mockTuner);

  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);

  controller.pushButton(remoteKey::KEY_3);
  controller.pushButton(remoteKey::KEY_4);
}

TEST(TVControllerTest, S1_4_InvalidateOnlyLastDigitAfterAutoChange) {
  MockTunerForController mockTuner;

  EXPECT_CALL(mockTuner, setCH("45")).Times(1);
  EXPECT_CALL(mockTuner, setCH("6")).Times(0);

  TVController controller(&mockTuner);

  // 실행
  controller.pushButton(remoteKey::KEY_4);
  controller.pushButton(remoteKey::KEY_5);

  controller.pushButton(remoteKey::KEY_6);
  controller.pushButton(remoteKey::KEY_MENU);

  // 확인을 위해 OK를 눌러도 아무 호출이 없어야 함
  controller.pushButton(remoteKey::KEY_OK);
}

TEST(TVControllerTest, S1_5_LeadingZeroShouldBeRemoved) {
  MockTunerForController mockTuner;

  // 0, 7을 누르면 "07"이 아니라 "7"이 호출되어야 함 (명세 기반)
  EXPECT_CALL(mockTuner, setCH("7")).Times(1);

  TVController controller(&mockTuner);

  // 0 입력 후 7 입력 -> 2자리가 찼으므로 자동 변경 발생
  controller.pushButton(remoteKey::KEY_0);
  controller.pushButton(remoteKey::KEY_7);
}