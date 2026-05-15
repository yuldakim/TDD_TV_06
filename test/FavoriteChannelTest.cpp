#include "TVController.h"
#include "Tuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

// 추가: 여기서 MockTuner를 정의해줘야 아래 TestFixture에서 쓸 수 있습니다.
class MockTunerForFav : public Tuner {
public:
  MOCK_METHOD(std::string, seekCH, (), (override));
  MOCK_METHOD(void, setCH, (const std::string &ch), (override));
  MOCK_METHOD(std::string, getCurrentCH, (), (override));
};

class FavoriteChannelTest : public Test {
protected:
  MockTunerForFav tuner; // 이름을 MockTunerForFav로 맞춤
  TVController *controller;

  void SetUp() override { controller = new TVController(&tuner); }

  void TearDown() override { delete controller; }
};

// S2-1: 현재 채널을 선호 채널 목록에 추가
TEST_F(FavoriteChannelTest, S2_1_AddCurrentChannelToFavorites) {
  // 1. Given: 현재 튜너의 채널이 "10"번이라고 설정됨
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("10"));

  // 2. When: 선호 채널 버튼(★) 클릭
  controller->pushButton(remoteKey::KEY_FAV);

  // 3. Then: 리스트를 가져와서 10이 들어있는지 확인 (가장 심플한 검증!)
  std::vector<int> favs = controller->getFavorites();
  ASSERT_EQ(1, favs.size()); // 목록에 1개가 있어야 함
  EXPECT_EQ(10, favs[0]);    // 그 값이 10이어야 함
}

TEST_F(FavoriteChannelTest, S2_2_ToggleFavoriteRemoveIfExisted) {
  // 1. Given: 현재 채널이 "7"번이라고 가정
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("7"));

  // 2. When: 첫 번째 클릭 (추가)
  controller->pushButton(remoteKey::KEY_FAV);
  ASSERT_EQ(1, controller->getFavorites().size()); // 일단 1개 들어간 것 확인

  // 3. When: 두 번째 클릭 (삭제/토글)
  controller->pushButton(remoteKey::KEY_FAV);

  // 4. Then: 목록이 다시 비어있어야 함!
  EXPECT_EQ(0, controller->getFavorites().size());
}

TEST_F(FavoriteChannelTest, S2_3_ComplexSequenceTest) {
  // 1. 12번 추가
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("12"));
  controller->pushButton(remoteKey::KEY_FAV);

  // 2. 8번 추가
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("8"));
  controller->pushButton(remoteKey::KEY_FAV);

  // 3. 37번 추가
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("37"));
  controller->pushButton(remoteKey::KEY_FAV);

  // 4. 8번 다시 눌러서 삭제 (토글)
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("8"));
  controller->pushButton(remoteKey::KEY_FAV);

  // 5. 6번 추가
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("6"));
  controller->pushButton(remoteKey::KEY_FAV);

  // 최종 검증
  std::vector<int> favs = controller->getFavorites();

  // 개수는 3개여야 함
  ASSERT_EQ(3, favs.size());

  // 명세: 항상 정렬 유지 -> {6, 12, 37} 순서여야 함
  EXPECT_EQ(6, favs[0]);
  EXPECT_EQ(12, favs[1]);
  EXPECT_EQ(37, favs[2]);
}