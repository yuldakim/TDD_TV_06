#include "TVController.h"
#include "Tuner.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class RecordingTunerForController : public Tuner {
public:
  std::string currentChannel = "0";
  std::vector<std::string> setHistory;

  std::string seekCH() override { return currentChannel; }

  void setCH(const std::string &ch) override {
    currentChannel = ch;
    setHistory.push_back(ch);
  }

  std::string getCurrentCH() override { return currentChannel; }
};

class TVControllerChannelChangeTest : public ::testing::Test {
protected:
  RecordingTunerForController tuner;
  TVController controller{&tuner};
};

TEST_F(TVControllerChannelChangeTest, OneDigitAndOkChangesToChannelOne) {
  // Given
  const std::vector<std::string> expectedHistory = {"1"};

  // When
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_OK);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("1", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, TwoDigitsChangeImmediately) {
  // Given
  const std::vector<std::string> expectedHistory = {"12"};

  // When
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("12", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, OkAfterTwoDigitAutoChangeDoesNothing) {
  // Given
  const std::vector<std::string> expectedHistory = {"12"};

  // When
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);
  controller.pushButton(remoteKey::KEY_OK);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("12", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, ContinuousDigitsChangeEveryTwoDigits) {
  // Given
  const std::vector<std::string> expectedHistory = {"12", "34"};

  // When
  controller.pushButton(remoteKey::KEY_1);
  controller.pushButton(remoteKey::KEY_2);
  controller.pushButton(remoteKey::KEY_3);
  controller.pushButton(remoteKey::KEY_4);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("34", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, MenuCancelsPendingOneDigitInput) {
  // Given
  const std::vector<std::string> expectedHistory = {};

  // When
  controller.pushButton(remoteKey::KEY_6);
  controller.pushButton(remoteKey::KEY_MENU);
  controller.pushButton(remoteKey::KEY_OK);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("0", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, MenuCancelsOnlyUncommittedDigit) {
  // Given
  const std::vector<std::string> expectedHistory = {"45"};

  // When
  controller.pushButton(remoteKey::KEY_4);
  controller.pushButton(remoteKey::KEY_5);
  controller.pushButton(remoteKey::KEY_6);
  controller.pushButton(remoteKey::KEY_MENU);
  controller.pushButton(remoteKey::KEY_OK);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("45", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, LeadingZeroIsNormalized) {
  // Given
  const std::vector<std::string> expectedHistory = {"7"};

  // When
  controller.pushButton(remoteKey::KEY_0);
  controller.pushButton(remoteKey::KEY_7);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("7", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, LowerBoundaryZeroCanBeSelectedByTwoZeros) {
  // Given
  const std::vector<std::string> expectedHistory = {"0"};

  // When
  controller.pushButton(remoteKey::KEY_0);
  controller.pushButton(remoteKey::KEY_0);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("0", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, LowerBoundaryZeroCanBeConfirmedByOk) {
  // Given
  const std::vector<std::string> expectedHistory = {"0"};

  // When
  controller.pushButton(remoteKey::KEY_0);
  controller.pushButton(remoteKey::KEY_OK);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("0", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, UpperBoundaryNinetyNineCanBeSelected) {
  // Given
  const std::vector<std::string> expectedHistory = {"99"};

  // When
  controller.pushButton(remoteKey::KEY_9);
  controller.pushButton(remoteKey::KEY_9);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("99", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, ChannelUpIncrementsCurrentChannel) {
  // Given
  tuner.currentChannel = "6";
  const std::vector<std::string> expectedHistory = {"7"};

  // When
  controller.pushButton(remoteKey::KEY_CH_UP);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("7", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, ChannelDownDecrementsCurrentChannel) {
  // Given
  tuner.currentChannel = "6";
  const std::vector<std::string> expectedHistory = {"5"};

  // When
  controller.pushButton(remoteKey::KEY_CH_DOWN);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("5", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, ChannelUpWrapsFromNinetyNineToOne) {
  // Given
  tuner.currentChannel = "99";
  const std::vector<std::string> expectedHistory = {"1"};

  // When
  controller.pushButton(remoteKey::KEY_CH_UP);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("1", tuner.currentChannel);
}

TEST_F(TVControllerChannelChangeTest, ChannelDownWrapsFromZeroToNinetyNine) {
  // Given
  tuner.currentChannel = "0";
  const std::vector<std::string> expectedHistory = {"99"};

  // When
  controller.pushButton(remoteKey::KEY_CH_DOWN);

  // Then
  ASSERT_EQ(expectedHistory, tuner.setHistory);
  EXPECT_EQ("99", tuner.currentChannel);
}
