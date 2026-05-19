#include "Tuner.h"
#include <gtest/gtest.h>
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

class TunerFakeTest : public ::testing::Test {
protected:
  FakeTunerForTunerTest tuner;
};

TEST_F(TunerFakeTest, SearchMovesFromInitialChannelToNextAvailableChannel) {
  // Given
  ASSERT_EQ("0", tuner.getCurrentCH());

  // When
  const std::string foundChannel = tuner.seekCH();

  // Then
  EXPECT_EQ("5", foundChannel);
  EXPECT_EQ("5", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, SearchCanBeRepeatedAndChangesChannelEachTime) {
  // Given
  const std::vector<std::string> expected = {"5", "10", "15"};
  std::vector<std::string> actual;

  // When
  actual.push_back(tuner.seekCH());
  actual.push_back(tuner.seekCH());
  actual.push_back(tuner.seekCH());

  // Then
  ASSERT_EQ(expected, actual);
  EXPECT_EQ("15", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, SearchWrapsAfterUpperBoundary) {
  // Given
  tuner.setCH("99");

  // When
  const std::string foundChannel = tuner.seekCH();

  // Then
  EXPECT_EQ("4", foundChannel);
  EXPECT_EQ("4", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, SearchResultAlwaysStaysWithinChannelBoundary) {
  // Given
  constexpr int searchCount = 25;

  // When & Then
  for (int i = 0; i < searchCount; ++i) {
    const int foundChannel = std::stoi(tuner.seekCH());
    EXPECT_EQ(true, 0 <= foundChannel && foundChannel <= 99);
  }
}

TEST_F(TunerFakeTest, SearchAfterExplicitChannelChangeUsesCurrentChannel) {
  // Given
  tuner.setCH("20");

  // When
  const std::string foundChannel = tuner.seekCH();

  // Then
  EXPECT_EQ("25", foundChannel);
  EXPECT_EQ("25", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, GetCurrentChannelReturnsInitialChannel) {
  // Given
  const std::string expectedChannel = "0";

  // When
  const std::string currentChannel = tuner.getCurrentCH();

  // Then
  EXPECT_EQ(expectedChannel, currentChannel);
}

TEST_F(TunerFakeTest, GetCurrentChannelReturnsLowerBoundaryAfterSet) {
  // Given
  tuner.setCH("0");

  // When
  const std::string currentChannel = tuner.getCurrentCH();

  // Then
  EXPECT_EQ("0", currentChannel);
}

TEST_F(TunerFakeTest, GetCurrentChannelReturnsUpperBoundaryAfterSet) {
  // Given
  tuner.setCH("99");

  // When
  const std::string currentChannel = tuner.getCurrentCH();

  // Then
  EXPECT_EQ("99", currentChannel);
}

TEST_F(TunerFakeTest, GetCurrentChannelReflectsLastValidSetChannel) {
  // Given
  const std::vector<std::string> validChannels = {"4", "5", "12"};

  // When
  for (const auto &channel : validChannels) {
    tuner.setCH(channel);
  }

  // Then
  EXPECT_EQ("12", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, GetCurrentChannelIsUnchangedAfterInvalidSetFails) {
  // Given
  tuner.setCH("12");

  // When
  EXPECT_THROW(tuner.setCH("100"), std::invalid_argument);

  // Then
  EXPECT_EQ("12", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, InvalidChannelBelowLowerBoundaryIsRejected) {
  // Given
  const std::string invalidChannel = "-1";

  // When
  EXPECT_THROW(tuner.setCH(invalidChannel), std::invalid_argument);

  // Then
  EXPECT_EQ("0", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, InvalidChannelAboveUpperBoundaryIsRejected) {
  // Given
  const std::string invalidChannel = "100";

  // When
  EXPECT_THROW(tuner.setCH(invalidChannel), std::invalid_argument);

  // Then
  EXPECT_EQ("0", tuner.getCurrentCH());
}

TEST_F(TunerFakeTest, ValidChannelsCanBeSetAndQueriedIndividually) {
  // Given
  const std::vector<std::string> validChannels = {"0", "4", "5", "12", "99"};

  // When & Then
  for (const auto &channel : validChannels) {
    tuner.setCH(channel);
    EXPECT_EQ(channel, tuner.getCurrentCH());
  }
}
