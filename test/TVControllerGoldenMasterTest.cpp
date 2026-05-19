#include "TVController.h"
#include "Tuner.h"
#include "remoteKey.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class RecordingTunerForGoldenMaster : public Tuner {
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

class TVControllerGoldenMasterTextTestFixture : public ::testing::Test {
protected:
  static std::string readFile(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
  }

  static void writeFile(const std::filesystem::path &path,
                        const std::string &contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    output << contents;
  }

  static std::string normalizeLineEndings(const std::string &text) {
    std::string normalized;
    normalized.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i) {
      if (text[i] == '\r') {
        if (i + 1 < text.size() && text[i + 1] == '\n') {
          continue;
        }
        normalized.push_back('\n');
        continue;
      }
      normalized.push_back(text[i]);
    }

    return normalized;
  }

  static bool shouldUpdateGoldenMaster() {
    const char *value = std::getenv("UPDATE_GOLDEN_MASTER");
    return value != nullptr && std::string(value) == "1";
  }

  template <typename T>
  static std::string formatVector(const std::vector<T> &values) {
    std::ostringstream output;
    output << "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (i > 0) {
        output << ", ";
      }
      output << values[i];
    }
    output << "]";
    return output.str();
  }

  static std::string runKeys(TVController &controller,
                             const std::vector<remoteKey> &keys) {
    std::ostringstream captured;
    auto *originalBuffer = std::cout.rdbuf(captured.rdbuf());

    for (const auto key : keys) {
      controller.pushButton(key);
    }

    std::cout.rdbuf(originalBuffer);
    return captured.str();
  }

  static void appendScenario(std::ostringstream &output,
                             const std::string &name,
                             const std::string &keys,
                             const std::string &console,
                             const RecordingTunerForGoldenMaster &tuner,
                             const TVController &controller) {
    output << "[" << name << "]\n";
    output << "Keys        : " << keys << "\n";
    output << "Console     :\n";
    output << (console.empty() ? "(empty)\n" : console);
    output << "Final CH    : " << tuner.currentChannel << "\n";
    output << "Set History : " << formatVector(tuner.setHistory) << "\n";
    output << "Favorites   : " << formatVector(controller.getFavorites()) << "\n";
    output << "-------------------------------------\n";
  }

  static std::string renderGoldenMasterOutput() {
    std::ostringstream output;
    output << "TVController Golden Master\n";
    output << "==========================\n\n";

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      const auto console =
          runKeys(controller, {remoteKey::KEY_1, remoteKey::KEY_OK});
      appendScenario(output, "one_digit_ok_commits_channel", "1 OK", console,
                     tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      const auto console =
          runKeys(controller, {remoteKey::KEY_1, remoteKey::KEY_2,
                               remoteKey::KEY_3, remoteKey::KEY_4});
      appendScenario(output, "continuous_digits_commit_every_two_keys",
                     "1 2 3 4", console, tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      const auto console =
          runKeys(controller, {remoteKey::KEY_0, remoteKey::KEY_7});
      appendScenario(output, "leading_zero_is_normalized", "0 7", console,
                     tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      const auto console =
          runKeys(controller, {remoteKey::KEY_4, remoteKey::KEY_5,
                               remoteKey::KEY_6, remoteKey::KEY_MENU,
                               remoteKey::KEY_OK});
      appendScenario(output, "menu_cancels_only_pending_digit", "4 5 6 MENU OK",
                     console, tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      for (const auto &channel : {"12", "8", "37", "8", "6"}) {
        tuner.currentChannel = channel;
        controller.pushButton(remoteKey::KEY_FAV);
      }
      appendScenario(output, "favorites_toggle_and_remain_sorted",
                     "FAV@12 FAV@8 FAV@37 FAV@8 FAV@6", "", tuner,
                     controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      for (const auto &channel : {"1", "4", "12", "56"}) {
        tuner.currentChannel = channel;
        controller.pushButton(remoteKey::KEY_FAV);
      }
      tuner.currentChannel = "6";
      const auto console = runKeys(controller, {remoteKey::KEY_NEXT_FAV});
      appendScenario(output, "next_favorite_moves_to_upper_channel",
                     "NEXT_FAV from 6", console, tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      TVController controller{&tuner};
      for (const auto &channel : {"1", "4", "12", "56"}) {
        tuner.currentChannel = channel;
        controller.pushButton(remoteKey::KEY_FAV);
      }
      tuner.currentChannel = "56";
      const auto console = runKeys(controller, {remoteKey::KEY_NEXT_FAV});
      appendScenario(output, "next_favorite_wraps_to_first_channel",
                     "NEXT_FAV from 56", console, tuner, controller);
    }

    {
      RecordingTunerForGoldenMaster tuner;
      tuner.currentChannel = "23";
      TVController controller{&tuner};
      const auto console = runKeys(controller, {remoteKey::KEY_NEXT_FAV});
      appendScenario(output, "next_favorite_without_favorites_is_noop",
                     "NEXT_FAV from 23", console, tuner, controller);
    }

    return output.str();
  }
};

TEST_F(TVControllerGoldenMasterTextTestFixture,
       should_match_approved_text_output_when_replaying_characterization_scenarios) {
  const std::filesystem::path approvedPath = GOLDEN_MASTER_APPROVED_FILE;
  const std::filesystem::path receivedPath = GOLDEN_MASTER_RECEIVED_FILE;
  const std::string actual = renderGoldenMasterOutput();

  if (shouldUpdateGoldenMaster()) {
    writeFile(approvedPath, actual);
    std::filesystem::remove(receivedPath);
    SUCCEED() << "Updated golden master: " << approvedPath.string();
    return;
  }

  writeFile(receivedPath, actual);
  ASSERT_TRUE(std::filesystem::exists(approvedPath))
      << "Approved file is missing. Generate it with "
         "`UPDATE_GOLDEN_MASTER=1 ctest -R TVControllerGoldenMasterTest`.";

  const std::string expected = normalizeLineEndings(readFile(approvedPath));
  const std::string normalizedActual = normalizeLineEndings(actual);
  EXPECT_EQ(expected, normalizedActual)
      << "Golden master output changed.\nApproved: " << approvedPath.string()
      << "\nReceived : " << receivedPath.string()
      << "\nReview the diff, then regenerate only if the behavior change is "
         "intentional.";

  if (expected == normalizedActual) {
    std::filesystem::remove(receivedPath);
  }
}
