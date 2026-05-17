#ifndef REMOTE_KEY_H
#define REMOTE_KEY_H

#include <string>

enum class remoteKey {
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_OK,
  KEY_MENU,
  KEY_FAV,
  KEY_NEXT_FAV
};

// ⭕ 구현은 던져버리고 순수하게 선언만 남깁니다. (inline 제거)
std::string to_string(remoteKey key);

#endif // REMOTE_KEY_H