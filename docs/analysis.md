# C++ TV Controller 프로젝트 구조 분석

## 1. 전체 구조

이 프로젝트는 셋탑박스/TV 리모컨 입력을 받아 채널을 변경하고 선호 채널을 관리하는 `TVController` 모듈을 TDD 방식으로 구현한 C++17 프로젝트이다. 빌드는 CMake 기반이며, 테스트 프레임워크로 GoogleTest와 GoogleMock을 `FetchContent`로 가져온다.

```text
TDD_TV_06/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── Tuner.h
│   ├── TVController.h
│   └── remoteKey.h
├── src/
│   ├── TVController.cpp
│   └── remoteKey.cpp
└── test/
    ├── TunerTest.cpp
    ├── TVControllerTest.cpp
    ├── FavoriteChannelTest.cpp
    ├── TunerTest.approved.txt
    ├── TVControllerTest.approved.txt
    └── FavoriteChannelTest.approved.txt
```

주요 제품 코드는 `include`와 `src`에 있고, 테스트 코드는 `test`에 분리되어 있다. `build/_deps` 아래에는 CMake가 내려받은 GoogleTest/GoogleMock 소스가 포함되어 있으므로 프로젝트 자체 구조 분석에서는 제외하는 것이 적절하다.

## 2. 주요 클래스와 책임

### `Tuner`

위치: `include/Tuner.h`

`Tuner`는 실제 채널 튜너 하드웨어 또는 외부 제공 컴포넌트를 추상화하는 순수 가상 인터페이스이다.

책임:

- 현재 채널에서 시청 가능한 다음 채널을 검색하고 변경하는 `seekCH()` 제공
- 지정 채널로 변경하는 `setCH(const std::string& ch)` 제공
- 현재 채널을 조회하는 `getCurrentCH()` 제공

특징:

- 제품 코드에는 실제 `Tuner` 구현체가 없다.
- `TVController`는 `Tuner`의 구체 구현을 알지 않고 인터페이스 포인터만 사용한다.
- 테스트에서는 이 인터페이스를 상속한 Fake 또는 Mock이 실제 튜너를 대체한다.

### `TVController`

위치: `include/TVController.h`, `src/TVController.cpp`

`TVController`는 리모컨 키 입력을 해석하고, 채널 변경 및 선호 채널 로직을 조정하는 핵심 애플리케이션 클래스이다.

내부 상태:

- `Tuner* tuner`: 채널 변경을 위임할 튜너 의존성
- `std::string processingCH`: 숫자 키 입력 중인 임시 채널 버퍼
- `std::vector<int> favorites`: 오름차순으로 유지되는 선호 채널 목록

책임:

- 숫자 키 입력을 누적해 한 자리 또는 두 자리 채널 변경으로 변환
- 두 자리 입력이 완성되면 자동으로 `Tuner::setCH()` 호출
- `KEY_OK` 입력 시 미완성 숫자 버퍼를 채널 변경으로 확정
- `KEY_MENU` 입력 시 미완성 숫자 버퍼를 폐기
- `KEY_FAV` 입력 시 현재 채널을 선호 채널에 추가하거나 이미 있으면 삭제
- `KEY_NEXT_FAV` 입력 시 현재 채널보다 큰 선호 채널 중 가장 가까운 채널로 이동하고, 없으면 첫 선호 채널로 순환

구현상 주의점:

- `pushButton(remoteKey key)` 안의 `switch`가 대부분의 입력 처리 분기를 담당한다.
- `setTunerCh()`는 `processingCH`를 기반으로 실제 `tuner->setCH()`를 호출하는 내부 헬퍼이다.
- `KEY_FAV`, `KEY_NEXT_FAV` 처리에서는 `tuner->getCurrentCH()`의 문자열 값을 `std::stoi()`로 변환한다.
- `favorites`는 추가 후 `std::sort()`로 정렬되고, 다음 선호 채널 검색은 `std::upper_bound()`를 사용한다.

### `remoteKey`

위치: `include/remoteKey.h`, `src/remoteKey.cpp`

`remoteKey`는 리모컨 입력 키를 표현하는 `enum class`이다.

현재 정의된 키:

- 숫자 키: `KEY_0` ~ `KEY_9`
- 확정/메뉴 키: `KEY_OK`, `KEY_MENU`
- 선호 채널 키: `KEY_FAV`, `KEY_NEXT_FAV`

`to_string(remoteKey key)`는 키를 문자열로 변환한다. 숫자 키는 `"0"` ~ `"9"`로 변환되어 `TVController::processingCH`에 누적되고, 그 외 키는 테스트/디버깅 또는 향후 확장을 위한 문자열 표현을 제공한다.

### 테스트 전용 클래스

#### `FakeTunerForTunerTest`

위치: `test/TunerTest.cpp`

`Tuner` 자체의 기대 동작을 검증하기 위해 만든 Fake이다. 실제 하드웨어 대신 내부 `int currentCh`를 사용한다.

책임:

- `setCH()`에서 0~99 범위 검증 후 현재 채널 갱신
- `seekCH()`에서 현재 채널을 5씩 증가시키고 100 기준으로 순환
- `getCurrentCH()`로 현재 채널 반환

#### `MockTunerForController`

위치: `test/TVControllerTest.cpp`

GoogleMock 기반 Mock이다. `TVController`가 특정 입력 시 `Tuner::setCH()`를 올바른 인자로 호출하는지 검증한다.

책임:

- `MOCK_METHOD`로 `seekCH`, `setCH`, `getCurrentCH` 호출을 관찰 가능하게 만든다.
- `EXPECT_CALL(mockTuner, setCH("..."))`로 채널 변경 명령의 발생 여부와 횟수를 검증한다.

#### `FakeTunerForFav`

위치: `test/FavoriteChannelTest.cpp`

선호 채널 기능 테스트를 위한 단순 Fake이다. `currentChannel` 문자열을 public 필드로 두고 테스트가 직접 현재 채널을 조작한다.

책임:

- `setCH()` 호출 시 `currentChannel` 갱신
- `getCurrentCH()`로 현재 채널 반환
- `seekCH()`는 별도 탐색 없이 현재 채널을 그대로 반환

## 3. 클래스 간 의존성 관계

핵심 의존성은 `TVController -> Tuner` 방향으로만 존재한다. `Tuner`는 `TVController`를 알지 못하며, 테스트 더블은 `Tuner` 인터페이스를 구현해 `TVController`에 주입된다.

```text
remoteKey enum
     |
     v
TVController -----> Tuner interface
     |                    ^
     |                    |
     |          +---------+----------------+
     |          |                          |
     v          v                          v
favorites  FakeTunerForFav      MockTunerForController
state      FakeTunerForTunerTest
```

세부 의존성:

- `TVController.h`는 `Tuner.h`, `remoteKey.h`, `<string>`, `<vector>`에 의존한다.
- `TVController.cpp`는 `TVController.h`, `<algorithm>`, `<iostream>`에 의존한다.
- `remoteKey.cpp`는 `remoteKey.h`에만 의존한다.
- `Tuner.h`는 `<string>`에만 의존하는 얇은 인터페이스이다.
- 테스트 파일들은 GoogleTest/GoogleMock과 제품 헤더를 포함한다.

의존성 설계의 장점:

- 실제 튜너 구현이 없어도 `TVController`의 대부분 동작을 검증할 수 있다.
- `Tuner`가 인터페이스로 분리되어 있어 테스트 더블 주입이 쉽다.
- 제품 코드가 GoogleTest/GoogleMock에 의존하지 않는다.

의존성 설계의 한계:

- `TVController`가 원시 포인터 `Tuner*`를 보관하므로 수명 관리는 호출자 책임이다.
- `TVController` 안에 숫자 입력, 선호 채널, 다음 선호 채널 정책이 모두 들어 있어 기능이 늘어나면 `pushButton()`이 커지기 쉽다.
- `TVController`가 `std::cout`에 직접 출력하므로 순수 비즈니스 로직과 콘솔 출력이 결합되어 있다.

## 4. State 패턴 구현 방식

현재 코드에는 GoF State 패턴의 전형적인 구조, 즉 `State` 추상 클래스와 `ConcreteState` 클래스들이 분리된 형태는 없다. 대신 `TVController` 내부의 `processingCH` 값과 `pushButton()`의 `switch` 분기가 암묵적인 상태 머신처럼 동작한다.

### 현재의 암묵적 상태

`TVController`의 입력 처리 상태는 주로 `processingCH`로 표현된다.

```text
Idle 상태
  processingCH == ""

DigitPending 상태
  processingCH == "0" ~ "9"

AutoCommit 직후
  processingCH 길이가 2가 되면 setCH() 호출 후 다시 ""
```

상태 전이:

```text
Idle
  -- 숫자 키 --> DigitPending
  -- FAV --> 현재 채널 선호 목록 토글
  -- NEXT_FAV --> 다음 선호 채널 이동 또는 무시
  -- MENU/OK --> 실질 변화 없음

DigitPending
  -- 숫자 키 --> 두 자리 채널 확정 후 Idle
  -- OK --> 한 자리 채널 확정 후 Idle
  -- MENU --> 입력 취소 후 Idle
  -- FAV/NEXT_FAV --> 현재 구현에서는 processingCH를 유지한 채 선호 채널 로직 수행
```

### State 패턴 관점의 평가

현재 구현은 State 패턴이라기보다 절차적 상태 분기 방식이다.

- 상태 객체가 없다.
- 상태별 책임이 클래스 단위로 분리되어 있지 않다.
- `pushButton()`이 모든 상태와 이벤트 조합을 직접 처리한다.
- 상태 전이는 `processingCH` 문자열 변경과 `break` 흐름으로 암묵적으로 표현된다.

그럼에도 작은 기능 범위에서는 단순하고 이해하기 쉽다는 장점이 있다. 현재 요구사항 중 숫자 입력, OK, MENU, FAV, NEXT_FAV 정도까지는 하나의 `switch`로 감당 가능하다.

다만 README에는 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운 같은 추가 요구사항이 남아 있다. 이 기능들이 구현되면 다음과 같은 상태가 필요해질 가능성이 높다.

- 일반 시청 상태
- 숫자 입력 대기 상태
- 채널 검색 결과 보유 상태
- 선호 채널 탐색 상태

이 경우 정식 State 패턴으로 전환하면 `TVController`는 현재 상태 객체에 이벤트를 위임하고, 각 상태 클래스가 자신에게 의미 있는 키만 처리하도록 분리할 수 있다.

예상 구조:

```text
TVController
  └── currentState: ControllerState

ControllerState
  ├── IdleState
  ├── DigitInputState
  ├── ScannedChannelState
  └── FavoriteNavigationState
```

하지만 현재 코드 기준으로는 아직 State 패턴을 도입하지 않은 상태이며, 문서화 시에는 "암묵적 상태 머신"으로 보는 것이 정확하다.

## 5. Test Double 사용 패턴

이 프로젝트는 외부 튜너 의존성을 직접 구현하지 않고 테스트 더블로 대체한다. 사용된 테스트 더블은 Fake와 Mock이다.

### Fake 패턴

Fake는 실제 동작에 가까운 단순 구현체를 만들어 테스트에 사용하는 방식이다.

사용 위치:

- `test/TunerTest.cpp`의 `FakeTunerForTunerTest`
- `test/FavoriteChannelTest.cpp`의 `FakeTunerForFav`

사용 목적:

- 현재 채널 값을 메모리에 저장해 실제 튜너 없이 상태 기반 테스트 수행
- 선호 채널 기능처럼 `getCurrentCH()`와 `setCH()`의 결과 상태를 확인해야 하는 테스트에 적합
- Approval Test에서 스냅샷 파일을 만들 때 일관된 상태를 제공

특징:

- `FakeTunerForTunerTest`는 범위 검증과 `seekCH()` 순환 로직까지 포함해 비교적 실제 동작을 흉내 낸다.
- `FakeTunerForFav`는 선호 채널 테스트에 필요한 최소 기능만 구현한다.
- Fake는 호출 횟수보다 최종 상태 검증에 초점을 둔다.

### Mock 패턴

Mock은 객체 간 상호작용, 특히 특정 메서드 호출 여부와 인자를 검증하는 방식이다.

사용 위치:

- `test/TVControllerTest.cpp`의 `MockTunerForController`

사용 목적:

- 숫자 키 입력 조합에 따라 `TVController`가 `Tuner::setCH()`를 정확히 호출하는지 검증
- `KEY_MENU` 이후 `KEY_OK`가 잘못된 채널 변경을 일으키지 않는지 `Times(0)`으로 검증
- 채널 변경 결과 상태보다 "명령을 보냈는가"를 확인

예시 검증 흐름:

```text
EXPECT_CALL(mockTuner, setCH("12")).Times(1)
controller.pushButton(KEY_1)
controller.pushButton(KEY_2)
```

이 테스트는 실제 채널 값을 저장하지 않아도 `TVController`가 튜너에 올바른 메시지를 보냈는지 확인한다.

### Approval Test 패턴

세 테스트 파일 모두 `.received.txt`를 생성하고 `.approved.txt`와 비교하는 승인 테스트 패턴도 함께 사용한다.

사용 위치:

- `TunerTest.approved.txt`
- `TVControllerTest.approved.txt`
- `FavoriteChannelTest.approved.txt`

사용 목적:

- 여러 시나리오의 결과를 텍스트 스냅샷으로 누적
- 최종 `ZZZ_...ApprovalVerification` 테스트에서 승인 파일과 수신 파일 비교
- 결과가 같으면 `.received.txt`를 삭제해 테스트 산출물을 정리

주의점:

- 승인 테스트는 실행 순서와 파일 경로에 민감하다.
- CMake에는 일부 working directory 설정이 있지만, 테스트별 상대 경로가 `../test/...`로 하드코딩되어 있다.
- `TunerTest.cpp`의 스냅샷 라벨 `Initial Channel`은 실제로 테스트 종료 시점의 현재 채널을 출력하므로 이름과 의미가 어긋날 수 있다.

## 6. 구현된 기능과 미구현 요구사항

현재 구현된 기능:

- 숫자 입력으로 채널 변경
- 한 자리 입력 후 `OK`로 확정
- 두 자리 입력 시 자동 확정
- 앞자리 0 제거
- `MENU`로 미완성 숫자 입력 취소
- 선호 채널 추가/삭제 토글
- 선호 채널 목록 오름차순 유지
- 다음 선호 채널 이동 및 wrap-around

README에 있으나 현재 제품 코드에 없는 기능:

- 채널 검색 버튼 동작
- 채널 업/다운 키
- 검색 결과가 없을 때의 업/다운 이동
- 검색 결과가 있을 때의 업/다운 이동

또한 `remoteKey` enum에도 채널 업/다운, 채널 검색에 대응하는 키가 아직 정의되어 있지 않다.

## 7. 설계 요약

이 프로젝트는 `TVController`를 중심으로 한 단순한 포트-어댑터 형태의 구조를 가진다. `Tuner`가 외부 하드웨어 포트 역할을 하고, 테스트에서는 Fake와 Mock이 이 포트를 대체한다. 현재 구현은 State 패턴을 명시적으로 사용하지 않고 `processingCH`와 `switch` 기반의 암묵적 상태 머신으로 리모컨 입력을 처리한다.

현재 규모에서는 단순성이 장점이지만, README의 남은 요구사항까지 확장하면 `pushButton()`의 분기가 빠르게 복잡해질 수 있다. 향후 채널 검색/업다운/검색 결과 기반 탐색까지 포함한다면 입력 상태를 별도 State 클래스로 분리하거나, 최소한 입력 버퍼 처리와 선호 채널 처리를 별도 정책 클래스로 나누는 리팩토링을 검토할 만하다.
