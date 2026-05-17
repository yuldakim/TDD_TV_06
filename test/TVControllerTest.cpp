#include "TVController.h"
#include "Tuner.h"
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

class MockTunerForController : public Tuner {
public:
  MOCK_METHOD(std::string, seekCH, (), (override));
  MOCK_METHOD(void, setCH, (const std::string &ch), (override));
  MOCK_METHOD(std::string, getCurrentCH, (), (override));
};

void verifyTVControllerApproval(const std::string &testName,
                                const std::string &finalTargetCh) {
  std::string receivedFileName = "../test/TVControllerTest.received.txt";

  std::ofstream recFile(receivedFileName, std::ios::app);
  recFile << "[" << testName << " Result]\n";
  recFile << "Expected Action Channel: " << finalTargetCh << "\n";
  recFile << "------------------------------------\n";
  recFile.close();
}

TEST(TVControllerTest, Press1AndConfirmShouldChangeToChannel1) {
  std::string receivedFileName = "../test/TVControllerTest.received.txt";
  std::remove(receivedFileName.c_str());

  MockTunerForController mockTuner;
  EXPECT_CALL(mockTuner, setCH("1")).Times(1);

  TVController controller(&mockTuner);
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_OK);

  verifyTVControllerApproval("S1_1_Press1AndConfirmShouldChangeToChannel1",
                             "1");
}

TEST(TVControllerTest, PressMultipleDigitsAndConfirmShouldChangeChannel) {
  MockTunerForController mockTuner;
  EXPECT_CALL(mockTuner, setCH("12")).Times(1);

  TVController controller(&mockTuner);
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);
  controller.pushButton(remoteKey::KEY_OK);

  verifyTVControllerApproval(
      "S1_2_PressMultipleDigitsAndConfirmShouldChangeChannel", "12");
}

TEST(TVControllerTest, S1_3_ContinuousInput) {
  MockTunerForController mockTuner;
  EXPECT_CALL(mockTuner, setCH("12")).Times(1);
  EXPECT_CALL(mockTuner, setCH("34")).Times(1);

  TVController controller(&mockTuner);
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);
  controller.pushButton(remoteKey::KEY_3);
  controller.pushButton(remoteKey::KEY_4);

  verifyTVControllerApproval("S1_3_ContinuousInput", "34");
}

TEST(TVControllerTest, S1_4_InvalidateOnlyLastDigitAfterAutoChange) {
  MockTunerForController mockTuner;
  EXPECT_CALL(mockTuner, setCH("45")).Times(1);
  EXPECT_CALL(mockTuner, setCH("6")).Times(0);

  TVController controller(&mockTuner);
  controller.pushButton(remoteKey::KEY_4);
  controller.pushButton(remoteKey::KEY_5);
  controller.pushButton(remoteKey::KEY_6);
  controller.pushButton(remoteKey::KEY_MENU);
  controller.pushButton(remoteKey::KEY_OK);

  verifyTVControllerApproval("S1_4_InvalidateOnlyLastDigitAfterAutoChange",
                             "45");
}

TEST(TVControllerTest, S1_5_LeadingZeroShouldBeRemoved) {
  MockTunerForController mockTuner;
  EXPECT_CALL(mockTuner, setCH("7")).Times(1);

  TVController controller(&mockTuner);
  controller.pushButton(remoteKey::KEY_0);
  controller.pushButton(remoteKey::KEY_7);

  verifyTVControllerApproval("S1_5_LeadingZeroShouldBeRemoved", "7");
}

TEST(TVControllerTest, ZZZ_FinalApprovalVerification) {
  std::string approvedFileName = "../test/TVControllerTest.approved.txt";
  std::string receivedFileName = "../test/TVControllerTest.received.txt";

  std::ofstream forceFlush(receivedFileName, std::ios::app);
  forceFlush.close();

  std::ifstream recFile(receivedFileName);
  std::stringstream recStream;
  recStream << recFile.rdbuf();
  recFile.close();

  std::ifstream appFile(approvedFileName);
  if (!appFile.is_open()) {
    FAIL() << "\n[Approval Alert] 통합 승인 "
              "파일(TVControllerTest.approved.txt)이 없습니다!\n"
           << "test/ 폴더에 새로 뽑힌 [TVControllerTest.received.txt] 내용을 "
              "검토하신 후,\n"
           << "문제가 없다면 파일명을 [TVControllerTest.approved.txt]로 변경해 "
              "주세요.\n";
  }

  std::stringstream appStream;
  appStream << appFile.rdbuf();
  appFile.close();

  if (appStream.str() == recStream.str()) {
    std::remove(receivedFileName.c_str());
  }

  EXPECT_EQ(appStream.str(), recStream.str());
}