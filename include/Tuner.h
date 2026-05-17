#ifndef TUNER_H
#define TUNER_H

#include <string>

// TV 채널 전환 및 탐색을 담당하는 하드웨어 인터페이스
// [제약조건] 채널 범위는 0 ~ 99이며, 범위를 벗어날 시 예외를 발생시킵니다.
class Tuner {
public:
  virtual ~Tuner() = default;

  virtual std::string seekCH() = 0;
  virtual void setCH(const std::string &ch) = 0;
  virtual std::string getCurrentCH() = 0;
};

#endif // TUNER_H