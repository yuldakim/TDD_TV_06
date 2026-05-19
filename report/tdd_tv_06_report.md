# TDD_TV_06 프로젝트 분석 및 Cursor 규칙 작성 보고서

## 1. 작업 개요

본 보고서는 `TDD_TV_06` C++17 프로젝트에 대해 수행한 코드 구조 분석과 Cursor AI 작업 규칙 작성 결과를 정리한다.

수행한 작업은 다음과 같다.

- C++ 프로젝트 전체 구조 분석
- 주요 클래스 역할과 책임 정리
- 클래스 간 의존성 관계 분석
- State 패턴 구현 여부 및 현재 상태 처리 방식 분석
- Test Double 사용 패턴 분석
- Cursor AI가 따라야 할 프로젝트 규칙을 `.cursorrules`로 작성
- 분석 결과와 대화 transcript를 별도 폴더에 보관

## 2. 프로젝트 기술 스택

- Language: C++17
- Build: CMake
- Test Framework: Google Test, Google Mock
- Test Style: Unit Test, Approval Test
- Coverage Context: gcov/lcov 사용 가능 구조

## 3. 프로젝트 구조 요약

프로젝트의 실제 제품 코드는 `include`와 `src`에 위치하며, 테스트 코드는 `test`에 위치한다.

```text
TDD_TV_06/
├── include/
│   ├── Tuner.h
│   ├── TVController.h
│   └── remoteKey.h
├── src/
│   ├── TVController.cpp
│   └── remoteKey.cpp
├── test/
│   ├── TunerTest.cpp
│   ├── TVControllerTest.cpp
│   └── FavoriteChannelTest.cpp
├── docs/
│   └── analysis.md
├── report/
│   └── tdd_tv_06_report.md
└── prompting/
    └── tdd_tv_06_report.md
```

`build/_deps` 아래의 GoogleTest/GoogleMock 소스와 빌드 산출물은 프로젝트 자체 구조 분석 범위에서 제외하였다.

## 4. 주요 클래스 역할과 책임

### `Tuner`

`Tuner`는 외부 튜너 또는 하드웨어를 추상화하는 순수 가상 인터페이스이다.

주요 책임:

- `seekCH()`로 시청 가능한 채널 탐색
- `setCH()`로 지정 채널 변경
- `getCurrentCH()`로 현재 채널 조회

제품 코드에는 실제 `Tuner` 구현체가 없으며, 테스트에서는 Fake 또는 Mock이 이 인터페이스를 구현한다.

### `TVController`

`TVController`는 리모컨 키 입력을 받아 채널 변경과 선호 채널 관리를 수행하는 핵심 클래스이다.

주요 책임:

- 숫자 키 입력을 임시 버퍼에 누적
- 한 자리 입력 후 `OK`로 채널 확정
- 두 자리 입력 시 자동 채널 확정
- `MENU` 입력으로 미완성 채널 입력 취소
- 현재 채널을 선호 채널 목록에 추가/삭제
- 다음 선호 채널로 이동하고 마지막 항목 이후에는 처음 항목으로 순환

### `remoteKey`

`remoteKey`는 리모컨 입력을 표현하는 `enum class`이다. 현재 구현된 키는 숫자 키, `KEY_OK`, `KEY_MENU`, `KEY_FAV`, `KEY_NEXT_FAV`이다.

## 5. 클래스 간 의존성 관계

핵심 의존성은 `TVController`가 `Tuner` 인터페이스에 의존하는 구조이다.

```text
remoteKey
   |
   v
TVController -----> Tuner
                       ^
                       |
          +------------+-------------+
          |            |             |
  FakeTunerForFav  FakeTunerForTunerTest  MockTunerForController
```

이 구조의 장점은 실제 튜너 구현 없이도 `TVController`의 동작을 테스트할 수 있다는 점이다. `Tuner`가 인터페이스로 분리되어 있어 Fake와 Mock 주입이 쉽고, 제품 코드는 GoogleTest/GoogleMock에 의존하지 않는다.

## 6. State 패턴 구현 방식

현재 코드에는 명시적인 GoF State 패턴 구조가 없다. 즉 `State` 추상 클래스와 `ConcreteState` 클래스 계층은 존재하지 않는다.

대신 `TVController` 내부의 `processingCH` 문자열과 `pushButton()`의 `switch` 분기가 암묵적인 상태 머신처럼 동작한다.

주요 상태는 다음과 같이 볼 수 있다.

- `Idle`: `processingCH == ""`
- `DigitPending`: 숫자 하나가 입력되어 `processingCH`에 남아 있는 상태
- `AutoCommit`: 두 자리 입력이 완성되어 `setCH()` 호출 후 다시 `Idle`로 돌아가는 흐름

따라서 현재 구현은 "명시적 State 패턴"이 아니라 "입력 버퍼 기반의 암묵적 상태 머신"으로 보는 것이 정확하다.

## 7. Test Double 사용 패턴

### Fake

`FakeTunerForTunerTest`와 `FakeTunerForFav`가 사용된다.

- 내부 상태를 저장해 실제 튜너처럼 동작한다.
- 최종 상태 검증에 적합하다.
- 선호 채널 기능처럼 현재 채널 값이 중요한 테스트에서 유용하다.

### Mock

`MockTunerForController`는 GoogleMock 기반 테스트 더블이다.

- `EXPECT_CALL`로 `setCH()` 호출 여부와 인자를 검증한다.
- 상태보다 상호작용 검증에 초점을 둔다.
- 숫자 키 입력 시 컨트롤러가 튜너에 올바른 명령을 보내는지 확인한다.

### Approval Test

각 테스트는 `.received.txt`를 생성하고 최종 검증 테스트에서 `.approved.txt`와 비교하는 승인 테스트 패턴을 함께 사용한다.

## 8. Cursor 규칙 작성 결과

프로젝트 루트에 `.cursorrules` 파일을 생성하였다.

규칙의 핵심 내용은 다음과 같다.

- C++17, CMake, Google Test, gcov/lcov 기반 작업
- 기존 코드의 동작 결과 보존을 절대 규칙으로 설정
- Given-When-Then 구조의 테스트 작성
- `TEST_F` 사용 권장
- 경계값 테스트 포함
- 테스트 메서드명은 `should_[result]_when_[condition]` 형식 사용
- 테스트 Green 상태에서만 리팩토링 진행
- 의미가 안정적인 매직 넘버는 상수화 권장

## 9. 산출물

- `docs/analysis.md`: 상세 코드 구조 분석 문서
- `.cursorrules`: Cursor AI 작업 규칙
- `report/tdd_tv_06_report.md`: 본 보고서
- `prompting/tdd_tv_06_report.md`: 현재 대화 transcript

## 10. 결론

현재 프로젝트는 `TVController`와 `Tuner` 인터페이스를 중심으로 작고 테스트 가능한 구조를 갖고 있다. State 패턴은 명시적으로 구현되어 있지 않지만, `processingCH`와 `switch` 기반의 상태 전이로 입력 처리를 수행한다. 테스트에서는 Fake, Mock, Approval Test를 함께 사용해 외부 튜너 의존성을 효과적으로 대체하고 있다.

향후 채널 검색, 채널 업/다운, 검색 결과 기반 이동 기능이 추가되면 `TVController::pushButton()`의 복잡도가 증가할 가능성이 높다. 이 시점에는 명시적 State 클래스 분리 또는 입력 처리 정책 분리를 검토하는 것이 적절하다.
