#include "remoteKey.h"

std::string to_string(remoteKey key) {
  switch (key) {
  case remoteKey::KEY_0:
    return "0";
  case remoteKey::KEY_1:
    return "1";
  case remoteKey::KEY_2:
    return "2";
  case remoteKey::KEY_3:
    return "3";
  case remoteKey::KEY_4:
    return "4";
  case remoteKey::KEY_5:
    return "5";
  case remoteKey::KEY_6:
    return "6";
  case remoteKey::KEY_7:
    return "7";
  case remoteKey::KEY_8:
    return "8";
  case remoteKey::KEY_9:
    return "9";
  case remoteKey::KEY_OK:
    return "OK";
  case remoteKey::KEY_MENU:
    return "MENU";
  case remoteKey::KEY_FAV:
    return "FAV";
  case remoteKey::KEY_NEXT_FAV:
    return "NEXT_FAV";
  }
  return "";
}