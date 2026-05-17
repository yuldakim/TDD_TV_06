#include "TVController.h"
#include "Tuner.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

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
  TVController *controller = nullptr;

  void SetUp() override { controller = new TVController(&tuner); }

  void TearDown() override { delete controller; }

public:
  void verifyApproval(const std::string &testName) {
    std::string receivedFileName = "../test/FavoriteChannelTest.received.txt";

    std::stringstream ss;
    auto favs = controller->getFavorites();
    ss << "[" << testName << " Snapshot]\n";
    ss << "Current Channel: " << tuner.currentChannel << "\n";
    ss << "Favorite List  : [";
    for (size_t i = 0; i < favs.size(); ++i) {
      ss << favs[i] << (i < favs.size() - 1 ? ", " : "");
    }
    ss << "]\n";
    ss << "-------------------------------------\n";

    std::ofstream recFile(receivedFileName, std::ios::app);
    recFile << ss.str();
    recFile.close();
  }
};

TEST_F(FavoriteChannelTest, S2_1_AddCurrentChannelToFavorites) {
  std::string receivedFileName = "../test/FavoriteChannelTest.received.txt";
  std::remove(receivedFileName.c_str());

  tuner.currentChannel = "10";
  controller->pushButton(remoteKey::KEY_FAV);

  std::vector<int> favs = controller->getFavorites();
  ASSERT_EQ(1, favs.size());
  EXPECT_EQ(10, favs[0]);

  verifyApproval("S2_1_AddCurrentChannelToFavorites");
}

TEST_F(FavoriteChannelTest, S2_2_ToggleFavoriteRemoveIfExisted) {
  tuner.currentChannel = "7";
  controller->pushButton(remoteKey::KEY_FAV);
  ASSERT_EQ(1, controller->getFavorites().size());

  controller->pushButton(remoteKey::KEY_FAV);
  EXPECT_EQ(0, controller->getFavorites().size());

  verifyApproval("S2_2_ToggleFavoriteRemoveIfExisted");
}

TEST_F(FavoriteChannelTest, S2_3_ComplexSequenceTest) {
  tuner.currentChannel = "12";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "8";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "37";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "8";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "6";
  controller->pushButton(remoteKey::KEY_FAV);

  std::vector<int> favs = controller->getFavorites();
  ASSERT_EQ(3, favs.size());
  EXPECT_EQ(6, favs[0]);
  EXPECT_EQ(12, favs[1]);
  EXPECT_EQ(37, favs[2]);

  verifyApproval("S2_3_ComplexSequenceTest");
}

TEST_F(FavoriteChannelTest, S3_1_NextFavoriteChannel_Normal) {
  tuner.currentChannel = "1";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "4";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "12";
  controller->pushButton(remoteKey::KEY_FAV);
  tuner.currentChannel = "56";
  controller->pushButton(remoteKey::KEY_FAV);

  tuner.currentChannel = "6";
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  EXPECT_EQ("12", tuner.currentChannel);

  verifyApproval("S3_1_NextFavoriteChannel_Normal");
}

TEST_F(FavoriteChannelTest, S3_2_NextFavoriteChannel_NoChange) {
  tuner.currentChannel = "7";
  controller->pushButton(remoteKey::KEY_FAV);

  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  EXPECT_EQ("7", tuner.currentChannel);

  verifyApproval("S3_2_NextFavoriteChannel_NoChange");
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

  tuner.currentChannel = "56";
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  EXPECT_EQ("1", tuner.currentChannel);

  verifyApproval("S3_3_NextFavoriteChannel_WrapAround");
}

TEST_F(FavoriteChannelTest, S3_4_NextFavoriteChannel_WhenListIsEmpty) {
  tuner.currentChannel = "23";
  controller->pushButton(remoteKey::KEY_NEXT_FAV);

  EXPECT_EQ("23", tuner.currentChannel);

  verifyApproval("S3_4_NextFavoriteChannel_WhenListIsEmpty");
}

TEST_F(FavoriteChannelTest, ZZZ_FinalFavoriteChannelApprovalVerification) {
  std::string approvedFileName = "../test/FavoriteChannelTest.approved.txt";
  std::string receivedFileName = "../test/FavoriteChannelTest.received.txt";

  std::ofstream forceFlush(receivedFileName, std::ios::app);
  forceFlush.close();

  std::ifstream recFile(receivedFileName);
  std::stringstream recStream;
  recStream << recFile.rdbuf();
  recFile.close();

  std::ifstream appFile(approvedFileName);
  if (!appFile.is_open()) {
    FAIL() << "\n[Approval Alert] 통합 승인 "
              "파일(FavoriteChannelTest.approved.txt)이 없습니다!\n"
           << "test/ 폴더에 새로 뽑힌 [FavoriteChannelTest.received.txt] "
              "내용을 검토하신 후,\n"
           << "문제가 없다면 파일명을 [FavoriteChannelTest.approved.txt]로 "
              "변경해 주세요.\n";
  }

  std::stringstream appStream;
  appStream << appFile.rdbuf();
  appFile.close();

  if (appStream.str() == recStream.str()) {
    std::remove(receivedFileName.c_str());
  }

  EXPECT_EQ(appStream.str(), recStream.str());
}