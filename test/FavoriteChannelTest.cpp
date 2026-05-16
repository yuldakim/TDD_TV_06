#include "TVController.h"
#include "Tuner.h"
#include <gtest/gtest.h>


// 1. 구글 목 대신 내부 변수에 채널 상태를 기억하는 순수 가짜 부품(Fake) 정의
class FakeTunerForFav : public Tuner {
public:
  std::string currentChannel = "0"; // 진짜 기계처럼 현재 채널 상태를 가짐

  void setCH(const std::string &ch) override {
    currentChannel = ch; // 채널 설정 명령이 오면 변수 값을 바꿈
  }

  std::string getCurrentCH() override {
    return currentChannel; // 현재 채널을 물어보면 변수 값을 그대로 반환
  }

  std::string seekCH() override { return currentChannel; }
};

class FavoriteChannelTest : public ::testing::Test {
protected:
  FakeTunerForFav tuner; // 가짜 튜너 부품 생성
  TVController *controller;

  void SetUp() override { controller = new TVController(&tuner); }

  void TearDown() override { delete controller; }
};

// --- S2: 선호 채널 관리 테스트 ---

TEST_F(FavoriteChannelTest, S2_1_AddCurrentChannelToFavorites) {
  tuner.currentChannel = "10"; // 튜너를 10번 채널로 직접 조작
  controller->pushButton(remoteKey::KEY_FAV);

  std::vector<int> favs = controller->getFavorites();
  ASSERT_EQ(1, favs.size());
  EXPECT_EQ(10, favs[0]);
}

TEST_F(FavoriteChannelTest, S2_2_ToggleFavoriteRemoveIfExisted) {
  tuner.currentChannel = "7";
  controller->pushButton(remoteKey::KEY_FAV); // 첫 번째 누름: 추가
  ASSERT_EQ(1, controller->getFavorites().size());

  controller->pushButton(remoteKey::KEY_FAV); // 두 번째 누름: 삭제(토글)
  EXPECT_EQ(0, controller->getFavorites().size());
}

TEST_F(FavoriteChannelTest, S2_3_ComplexSequenceTest) {
  // 시나리오: 12(추가) -> 8(추가) -> 37(추가) -> 8(삭제) -> 6(추가)
  tuner.currentChannel = "12";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "8";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "37";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "8";
  controller->pushButton(remoteKey::KEY_FAV); // 토글 삭제
  tuner.currentChannel = "6";
  controller->pushButton(remoteKey::KEY_FAV);

  std::vector<int> favs = controller->getFavorites();
  ASSERT_EQ(3, favs.size());
  EXPECT_EQ(6, favs[0]);
  EXPECT_EQ(12, favs[1]);
  EXPECT_EQ(37, favs[2]);
}

// --- S3: 다음 선호 채널 탐색 테스트 ---

TEST_F(FavoriteChannelTest, S3_1_NextFavoriteChannel_Normal) {
  // Given: 목록에 {1, 4, 12, 56}이 채워진 상태 만들기
  tuner.currentChannel = "1";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "4";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "12";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "56";
  controller->pushButton(remoteKey::KEY_FAV);

  // [핵심 조건] 현재 6번 시청 중
  tuner.currentChannel = "6";

  // When: 다음 선호 버튼 클릭
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  // Then: 6번보다 큰 다음 채널인 '12번'으로 튜너 상태가 조작되었는지 검증!
  EXPECT_EQ("12", tuner.currentChannel);
}

TEST_F(FavoriteChannelTest, S3_3_NextFavoriteChannel_WrapAround) {
  tuner.currentChannel = "1";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "4";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "12";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "56";
  controller->pushButton(remoteKey::KEY_FAV);

  // [핵심 조건] 현재 목록의 가장 마지막인 56번 시청 중
  tuner.currentChannel = "56";

  // When: 다음 선호 버튼 클릭
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  // Then: 다시 처음인 '1번'으로 되돌아왔는지 검증!
  EXPECT_EQ("1", tuner.currentChannel);
}

TEST_F(FavoriteChannelTest, S3_4_NextFavoriteChannel_WhenListIsEmpty) {
  // Given: 목록이 완전히 비어있고 현재 23번 시청 중
  tuner.currentChannel = "23";

  // When: 다음 선호 버튼 클릭
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  // Then: 채널 변경 없이 '23번' 상태가 그대로 유지되어야 함!
  EXPECT_EQ("23", tuner.currentChannel);
}