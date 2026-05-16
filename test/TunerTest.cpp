#include "Tuner.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>


// 1. TunerTest만을 위한 전용 FakeTuner 구현
// 실제 하드웨어의 '0~99 범위 제한' 및 '탐색(seek) 시 5씩 증가' 등의 스펙을
// 단순화하여 흉내 냅니다.
class FakeTunerForTunerTest : public Tuner {
private:
  int currentCh = 0; // 기본 초기값 0

public:
  void setCH(const std::string &ch) override {
    int chNum = std::stoi(ch);

    // [명세 반영] 0~99 범위를 벗어나면 예외(std::invalid_argument)를 던짐
    if (chNum < 0 || chNum > 99) {
      throw std::invalid_argument("Invalid channel range");
    }
    currentCh = chNum;
  }

  std::string getCurrentCH() override { return std::to_string(currentCh); }

  // [명세 반영] 호출될 때마다 채널을 일정 간격(예: 5)으로 이동시키는 탐색
  // 시뮬레이션
  std::string seekCH() override {
    currentCh = (currentCh + 5) % 100; // 99를 넘어가면 순환(Wrap-around)
    return std::to_string(currentCh);
  }
};

// 2. 테스트 픽스처 설정
class TunerFakeTest : public ::testing::Test {
protected:
  FakeTunerForTunerTest tuner;
};

// --- 기본 기능 테스트 ---

// Test 1: 초기 채널값 확인 (초기화 시 0번 채널이고, 0~99 범위 안에 있는가)
TEST_F(TunerFakeTest, initChannel) {
  int initCh = std::stoi(tuner.getCurrentCH());
  EXPECT_EQ(0, initCh);
  EXPECT_TRUE(initCh >= 0 && initCh <= 99);
}

// Test 2: 정상적인 채널 설정 경계값 테스트 (0, 4, 5, 12, 99)
TEST_F(TunerFakeTest, testSetChForValidChannel) {
  std::vector<std::string> validChannels = {"0", "4", "5", "12", "99"};

  for (const auto &ch : validChannels) {
    tuner.setCH(ch);
    EXPECT_EQ(ch, tuner.getCurrentCH());
  }
}

// Test 3: 비정상적인 채널 설정 예외 테스트 (-12, 100, 9999 처럼 범위를 벗어날
// 때)
TEST_F(TunerFakeTest, testSetChForInvalidChannel) {
  std::vector<std::string> invalidChannels = {"-12", "100", "9999"};

  for (const auto &ch : invalidChannels) {
    // 범위를 벗어난 채널 입력 시 예외가 올바르게 발생하는지 검증
    EXPECT_THROW(tuner.setCH(ch), std::invalid_argument);
  }
}

// Test 4: 채널 검색(seek) 기능 연속 10회 호출 테스트
TEST_F(TunerFakeTest, testSeekCh10times) {
  std::vector<std::string> seekChannels;

  for (int i = 0; i < 10; i++) {
    std::string seekCh = tuner.seekCH();
    int ch = std::stoi(seekCh);

    // 탐색된 채널이 언제나 0~99 범위 안에 있는지 확인
    EXPECT_TRUE(0 <= ch && ch <= 99);
    seekChannels.push_back(seekCh);
  }
  // 정확히 10번의 탐색 결과가 쌓였는지 확인
  EXPECT_EQ(10u, seekChannels.size());
}

// Test 5: 시작 채널을 99로 지정한 후 채널 검색 연속 10회 테스트 (순환 확인)
TEST_F(TunerFakeTest, testSeekCh10timesAfterSetCH) {
  tuner.setCH("99"); // 99번 경계선에서 시작

  std::vector<std::string> seekChannels;
  for (int i = 0; i < 10; i++) {
    std::string seekCh = tuner.seekCH();
    int ch = std::stoi(seekCh);

    EXPECT_TRUE(0 <= ch && ch <= 99);
    seekChannels.push_back(seekCh);
  }
  EXPECT_EQ(10u, seekChannels.size());
}