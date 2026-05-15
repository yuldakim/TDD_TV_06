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