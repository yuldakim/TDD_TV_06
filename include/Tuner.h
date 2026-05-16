#ifndef TUNER_H
#define TUNER_H

#include <string>

/**
 * @brief TV의 채널 전환 및 탐색을 담당하는 하드웨어 인터페이스
 * * [요구사항 및 제약조건]
 * 1. 채널 범위: 0 ~ 99 (범위를 벗어난 값 설정 시 예외 발생)
 * 2. 채널 검색(seekCH): 호출 시마다 다음 채널을 탐색하여 반환
 */
class Tuner {
public:
  // 가상 소멸자: 상속받는 자식 객체(FakeTuner 등)가 안전하게 해제되도록 보장
  virtual ~Tuner() = default;

  /**
   * @brief 채널 검색 기능
   * @return 탐색된 다음 채널 번호 (문자열 형태)
   */
  virtual std::string seekCH() = 0;

  /**
   * @brief 특정 채널로 강제 설정
   * @param ch 설정하고자 하는 채널 번호 (문자열 형태, "0" ~ "99")
   * @throws std::invalid_argument 채널 범위를 벗어나거나 올바르지 않은 입력일
   * 경우
   */
  virtual void setCH(const std::string &ch) = 0;

  /**
   * @brief 현재 시청 중인 채널 반환
   * @return 현재 채널 번호 (문자열 형태)
   */
  virtual std::string getCurrentCH() = 0;
};

#endif // TUNER_H