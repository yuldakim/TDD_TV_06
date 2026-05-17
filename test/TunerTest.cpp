#include "Tuner.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class FakeTunerForTunerTest : public Tuner {
private:
  int currentCh = 0;

public:
  void setCH(const std::string &ch) override {
    int chNum = std::stoi(ch);
    if (chNum < 0 || chNum > 99) {
      throw std::invalid_argument("Invalid channel range");
    }
    currentCh = chNum;
  }

  std::string getCurrentCH() override { return std::to_string(currentCh); }

  std::string seekCH() override {
    currentCh = (currentCh + 5) % 100;
    return std::to_string(currentCh);
  }
};

// 2. 테스트 픽스처 설정
class TunerFakeTest : public ::testing::Test {
protected:
  FakeTunerForTunerTest tuner;

public:
  void verifyTunerApproval(const std::string &testName,
                           const std::vector<std::string> &seekHistory) {
    std::string receivedFileName = "../test/TunerTest.received.txt";

    std::stringstream ss;
    ss << "[" << testName << " History Snapshot]\n";
    ss << "Initial Channel: " << tuner.getCurrentCH() << "\n";
    ss << "Seek Sequence  : ";
    for (size_t i = 0; i < seekHistory.size(); ++i) {
      ss << seekHistory[i];
      if (i < seekHistory.size() - 1)
        ss << " -> ";
    }
    ss << "\n-------------------------------------\n";

    std::ofstream recFile(receivedFileName, std::ios::app);
    recFile << ss.str();
    recFile.close();
  }
};

TEST_F(TunerFakeTest, initChannel) {
  int initCh = std::stoi(tuner.getCurrentCH());
  EXPECT_EQ(0, initCh);
  EXPECT_TRUE(initCh >= 0 && initCh <= 99);
}

TEST_F(TunerFakeTest, testSetChForValidChannel) {
  std::vector<std::string> validChannels = {"0", "4", "5", "12", "99"};

  for (const auto &ch : validChannels) {
    tuner.setCH(ch);
    EXPECT_EQ(ch, tuner.getCurrentCH());
  }
}

TEST_F(TunerFakeTest, testSetChForInvalidChannel) {
  std::vector<std::string> invalidChannels = {"-12", "100", "9999"};

  for (const auto &ch : invalidChannels) {
    EXPECT_THROW(tuner.setCH(ch), std::invalid_argument);
  }
}

TEST_F(TunerFakeTest, testSeekCh10times) {
  std::string receivedFileName = "../test/TunerTest.received.txt";
  std::remove(receivedFileName.c_str());

  std::vector<std::string> seekChannels;

  for (int i = 0; i < 10; i++) {
    std::string seekCh = tuner.seekCH();
    int ch = std::stoi(seekCh);

    EXPECT_TRUE(0 <= ch && ch <= 99);
    seekChannels.push_back(seekCh);
  }
  EXPECT_EQ(10u, seekChannels.size());

  verifyTunerApproval("testSeekCh10times", seekChannels);
}

TEST_F(TunerFakeTest, testSeekCh10timesAfterSetCH) {
  tuner.setCH("99");

  std::vector<std::string> seekChannels;
  for (int i = 0; i < 10; i++) {
    std::string seekCh = tuner.seekCH();
    int ch = std::stoi(seekCh);

    EXPECT_TRUE(0 <= ch && ch <= 99);
    seekChannels.push_back(seekCh);
  }
  EXPECT_EQ(10u, seekChannels.size());

  verifyTunerApproval("testSeekCh10timesAfterSetCH", seekChannels);
}

TEST_F(TunerFakeTest, ZZZ_FinalTunerApprovalVerification) {
  std::string approvedFileName = "../test/TunerTest.approved.txt";
  std::string receivedFileName = "../test/TunerTest.received.txt";

  std::ofstream forceFlush(receivedFileName, std::ios::app);
  forceFlush.close();

  std::ifstream recFile(receivedFileName);
  std::stringstream recStream;
  recStream << recFile.rdbuf();
  recFile.close();

  std::ifstream appFile(approvedFileName);
  if (!appFile.is_open()) {
    FAIL()
        << "\n[Approval Alert] 통합 승인 파일(TunerTest.approved.txt)이 "
           "없습니다!\n"
        << "test/ 폴더에 새로 뽑힌 [TunerTest.received.txt] 내용을 검토하신 "
           "후,\n"
        << "문제가 없다면 파일명을 [TunerTest.approved.txt]로 변경해 주세요.\n";
  }

  std::stringstream appStream;
  appStream << appFile.rdbuf();
  appFile.close();

  if (appStream.str() == recStream.str()) {
    std::remove(receivedFileName.c_str());
  }

  EXPECT_EQ(appStream.str(), recStream.str());
}