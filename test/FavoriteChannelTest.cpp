#include "TVController.h"
#include "Tuner.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class FakeTunerForFav : public Tuner {
public:
  std::string currentChannel = "0";

  void setCH(const std::string &ch) override { currentChannel = ch; }

  std::string getCurrentCH() override { return currentChannel; }

  std::string seekCH() override { return currentChannel; }
};

class FavoriteChannelTest : public ::testing::Test {
protected:
  FakeTunerForFav tuner;
  TVController controller{&tuner};

  void toggleFavoriteAt(const std::string &channel) {
    tuner.currentChannel = channel;
    controller.pushButton(remoteKey::KEY_FAV);
  }

  void addFavorites(const std::vector<std::string> &channels) {
    for (const auto &channel : channels) {
      toggleFavoriteAt(channel);
    }
  }
};

TEST_F(FavoriteChannelTest, S2_1_AddCurrentChannelToFavorites) {
  // Given
  tuner.currentChannel = "10";

  // When
  controller.pushButton(remoteKey::KEY_FAV);

  // Then
  const std::vector<int> expected = {10};
  ASSERT_EQ(expected, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S2_2_ToggleFavoriteRemoveIfExisted) {
  // Given
  toggleFavoriteAt("7");
  ASSERT_EQ(std::vector<int>{7}, controller.getFavorites());

  // When
  controller.pushButton(remoteKey::KEY_FAV);

  // Then
  EXPECT_EQ(std::vector<int>{}, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S2_3_ComplexSequenceTest) {
  // Given
  const std::vector<int> expected = {6, 12, 37};

  // When
  addFavorites({"12", "8", "37", "8", "6"});

  // Then
  ASSERT_EQ(expected, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S2_4_FavoritesRemainSortedAfterReverseInsertion) {
  // Given
  const std::vector<int> expected = {3, 21, 99};

  // When
  addFavorites({"99", "21", "3"});

  // Then
  ASSERT_EQ(expected, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S2_5_BoundaryChannelsCanBeFavorites) {
  // Given
  const std::vector<int> expected = {0, 99};

  // When
  addFavorites({"0", "99"});

  // Then
  ASSERT_EQ(expected, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S2_6_RemovingOneFavoriteKeepsOthersSorted) {
  // Given
  addFavorites({"30", "10", "20"});
  ASSERT_EQ((std::vector<int>{10, 20, 30}), controller.getFavorites());

  // When
  toggleFavoriteAt("20");

  // Then
  EXPECT_EQ((std::vector<int>{10, 30}), controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_1_NextFavoriteChannel_Normal) {
  // Given
  addFavorites({"1", "4", "12", "56"});
  tuner.currentChannel = "6";

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("12", tuner.currentChannel);
  EXPECT_EQ((std::vector<int>{1, 4, 12, 56}), controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_2_NextFavoriteChannel_NoChange) {
  // Given
  toggleFavoriteAt("7");

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("7", tuner.currentChannel);
  EXPECT_EQ(std::vector<int>{7}, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_3_NextFavoriteChannel_WrapAround) {
  // Given
  addFavorites({"1", "4", "12", "56"});
  tuner.currentChannel = "56";

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("1", tuner.currentChannel);
  EXPECT_EQ((std::vector<int>{1, 4, 12, 56}), controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_4_NextFavoriteChannel_WhenListIsEmpty) {
  // Given
  tuner.currentChannel = "23";

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("23", tuner.currentChannel);
  EXPECT_EQ(std::vector<int>{}, controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_5_NextFavoriteFromValueBelowAllMovesToFirst) {
  // Given
  addFavorites({"10", "20", "30"});
  tuner.currentChannel = "0";

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("10", tuner.currentChannel);
  EXPECT_EQ((std::vector<int>{10, 20, 30}), controller.getFavorites());
}

TEST_F(FavoriteChannelTest, S3_6_NextFavoriteFromGapMovesToUpperChannel) {
  // Given
  addFavorites({"10", "20", "30"});
  tuner.currentChannel = "15";

  // When
  controller.pushButton(remoteKey::KEY_NEXT_FAV);

  // Then
  EXPECT_EQ("20", tuner.currentChannel);
  EXPECT_EQ((std::vector<int>{10, 20, 30}), controller.getFavorites());
}
