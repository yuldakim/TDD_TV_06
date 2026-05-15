#include "TVController.h"
#include "Tuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

// 선호 채널 전용 테스트 피스처
class FavoriteChannelTest : public Test {
protected:
  MockTuner tuner; // TunerTest에서 정의한 MockTuner 사용
  TVController *controller;

  void SetUp() override { controller = new TVController(&tuner); }

  void TearDown() override { delete controller; }
};

// S2-1: 선호 채널 추가 테스트
TEST_F(FavoriteChannelTest, S2_1_AddFavorite) {
  // 현재 채널이 10번이라고 가정
  EXPECT_CALL(tuner, getCurrentCH()).WillRepeatedly(Return("10"));

  controller->pushButton(remoteKey::KEY_FAVORITE);

  // 검증 로직 (S3 기능을 이용해 목록에 들어갔는지 확인)
}