# `TVController::pushButton()` Code Quality Report

## 분석 대상

- 파일: `include/TVController.h`, `src/TVController.cpp`
- 함수: `TVController::pushButton(remoteKey key)`
- 관점: SOLID, Code Smell, C++17 리팩토링 가능성

## 문제점 분석

| 문제점 | 위반 원칙/스멜 | 영향 | 개선 방향 | 우선순위 |
|---|---|---|---|---|
| `pushButton()`가 숫자 입력 누적, 채널 확정, 입력 취소, 즐겨찾기 토글, 다음 즐겨찾기 이동을 모두 처리한다. | SRP 위반, Long Method | 리모컨 키가 추가될수록 함수가 계속 커지고, 채널 입력 정책과 즐겨찾기 정책을 독립적으로 테스트하거나 변경하기 어렵다. | `handleDigitKey()`, `commitProcessingChannel()`, `clearProcessingChannel()`, `toggleFavoriteChannel()`, `moveToNextFavoriteChannel()`처럼 행위 단위의 private 메서드로 분리한다. | 1 |
| `switch (key)`가 모든 키 동작을 중앙에서 직접 알고 있다. | OCP 위반, 조건문 복잡도 | 새 키를 추가할 때마다 `pushButton()` 수정이 필요하다. 수정 범위가 한 곳에 집중되어 회귀 위험이 커진다. | 키별 핸들러 테이블(`std::unordered_map<remoteKey, std::function<void()>>`) 또는 전략 객체를 두어 키 추가 시 기존 분기문 수정 없이 확장하도록 만든다. | 2 |
| 숫자 키 10개가 같은 동작을 수행하지만 `case`가 반복되어 있다. | Duplicated Code, Switch Statements | 숫자 키 판별 로직이 길게 나열되어 가독성이 낮고, 숫자 키 범위 정책이 암묵적으로 흩어진다. | `isDigitKey(remoteKey)`와 `digitOf(remoteKey)` helper를 만들거나, `remoteKey`의 enum 값 연속성을 이용하지 않는 명시적 변환 테이블을 둔다. | 3 |
| 두 자리 입력 완성 기준인 `2`가 코드에 직접 박혀 있다. | Magic Number | 채널 번호 자릿수 정책의 의미가 드러나지 않고, 3자리 채널 등 요구사항 변경 시 검색/수정 지점이 불명확하다. | `constexpr std::size_t kMaxChannelDigits = 2;` 또는 `MaxChannelDigits` 같은 도메인 상수로 분리한다. | 4 |
| `processingCH = ""` 초기화가 숫자 입력, OK, MENU 처리에서 반복된다. | Duplicated Code, Primitive Obsession | 버퍼 초기화 규칙이 여러 분기에 분산되어 이후 로깅, 검증, 상태 전이 변경 시 누락 가능성이 생긴다. | `clearProcessingChannel()` helper로 의도를 드러내고, 채널 확정 후 초기화 흐름을 `commitProcessingChannel()` 내부로 모은다. | 5 |
| 현재 채널 문자열을 여러 분기에서 `std::stoi(tuner->getCurrentCH())`로 직접 변환한다. | SRP 경계 흐림, Primitive Obsession | `Tuner`의 문자열 표현과 즐겨찾기 내부 표현이 `pushButton()`에 노출된다. 변환 실패 예외 처리 정책도 명확하지 않다. | `currentChannelNumber()` helper를 두거나 `Channel` 값 객체를 도입해 문자열/정수 변환 책임을 한 곳으로 모은다. | 3 |
| `KEY_NEXT_FAV` 분기에서 `upper_bound` 결과에 따라 다음 채널 또는 첫 채널로 순환하는 정책이 직접 구현되어 있다. | SRP 위반, 조건문 복잡도 | 즐겨찾기 순회 정책이 컨트롤러 입력 처리 로직에 묻혀 있어 독립 테스트가 어렵다. | `FavoriteChannels` 또는 `FavoriteChannelStore` 클래스로 추출하고 `nextAfter(curr)` 같은 도메인 메서드로 캡슐화한다. | 2 |
| `setTunerCh()`는 `processingCH` 내부 상태에 강하게 의존한다. | Hidden Dependency, 낮은 응집도 | 함수 이름은 채널 설정처럼 보이지만 실제로는 현재 입력 버퍼를 읽는다. 호출 전후 상태 조건을 모르면 오용하기 쉽다. | `setTunerChannel(std::string channel)`처럼 값을 인자로 받게 하거나 `commitProcessingChannel()`이 버퍼 읽기, 정규화, 튜너 호출, 버퍼 초기화를 모두 책임지게 한다. | 4 |

## SRP/OCP 관점 핵심 근거

`pushButton()`의 현재 책임은 "리모컨 입력을 받는다"보다 넓다. 숫자 키 입력 버퍼링, 채널 번호 정규화, 튜너 호출, 즐겨찾기 목록 변경, 즐겨찾기 순회까지 모두 포함한다. 이 책임들은 변경 이유가 다르다. 예를 들어 채널 자릿수 정책 변경, 즐겨찾기 정렬 정책 변경, 새 리모컨 키 추가는 서로 다른 요구사항이지만 모두 같은 함수 수정을 요구한다.

OCP 측면에서도 새 키가 추가될 때 `switch`에 `case`를 추가해야 한다. 현재 규모에서는 단순한 `switch`가 허용 가능한 선택이지만, 키 종류와 상태 전이가 늘어나는 순간 기존 함수를 계속 수정하는 구조가 된다. 따라서 입력 디스패치와 키별 동작을 분리하는 방향이 적절하다.

## Magic Number 상수화 필요성

현재 `processingCH.length() == 2`의 `2`는 "채널 입력은 최대 두 자리까지 자동 확정한다"는 도메인 정책이다. 이 값은 단순 계산 상수가 아니라 사용자 입력 UX와 채널 범위를 결정한다. 따라서 다음처럼 의미 있는 이름을 부여하는 것이 좋다.

```cpp
namespace {
constexpr std::size_t MaxAutoCommitChannelDigits = 2;
}
```

상수화하면 정책 변경 시 수정 지점이 명확해지고, 테스트 이름과 요구사항 문서에서도 같은 개념으로 추적하기 쉬워진다.

## C++17 스타일 개선 방향

### 1. 우선 적용 가능한 함수 추출

가장 안전한 1차 리팩토링은 동작을 바꾸지 않고 private helper로 책임을 나누는 것이다.

- `isDigitKey(remoteKey key)`
- `appendDigit(remoteKey key)`
- `commitProcessingChannel()`
- `clearProcessingChannel()`
- `toggleFavoriteChannel()`
- `moveToNextFavoriteChannel()`

이 방식은 기존 테스트를 유지하면서 Long Method와 중복 초기화를 줄일 수 있다.

### 2. 테이블 기반 디스패치

키 종류가 늘어날 가능성이 있다면 `switch`를 키별 handler 테이블로 옮길 수 있다.

```cpp
using KeyHandler = std::function<void()>;
const std::unordered_map<remoteKey, KeyHandler> handlers = {
    {remoteKey::KEY_OK, [this] { commitProcessingChannel(); }},
    {remoteKey::KEY_MENU, [this] { clearProcessingChannel(); }},
    {remoteKey::KEY_FAV, [this] { toggleFavoriteChannel(); }},
    {remoteKey::KEY_NEXT_FAV, [this] { moveToNextFavoriteChannel(); }},
};
```

숫자 키는 별도 `isDigitKey()`로 선처리하거나, 숫자 키까지 테이블에 등록할 수 있다. 다만 매 호출마다 테이블을 생성하지 않도록 멤버 초기화 또는 정적 구성 방식을 고려해야 한다.

### 3. 전략 패턴

키별 동작이 더 복잡해지고 상태 전이가 많아진다면 `RemoteKeyCommand` 또는 `KeyAction` 전략 객체로 분리할 수 있다. 각 전략은 `TVController`의 제한된 인터페이스를 통해 필요한 작업만 수행하게 만들면 OCP에 가장 잘 맞는다. 단, 현재 규모에서는 클래스 수 증가가 과할 수 있으므로 1차 리팩토링 이후 필요성이 확인될 때 적용하는 편이 좋다.

### 4. `std::variant` 기반 이벤트 모델

`remoteKey`를 그대로 모든 분기의 기준으로 삼기보다, 입력 해석 계층에서 다음과 같은 의미 있는 이벤트로 변환할 수 있다.

```cpp
struct DigitPressed { int digit; };
struct OkPressed {};
struct MenuPressed {};
struct FavoritePressed {};
struct NextFavoritePressed {};

using RemoteEvent = std::variant<
    DigitPressed,
    OkPressed,
    MenuPressed,
    FavoritePressed,
    NextFavoritePressed
>;
```

이 접근은 "물리 키"와 "도메인 이벤트"를 분리한다. `std::visit`로 이벤트별 처리를 구성하면 숫자 키 10개의 중복 분기를 줄이고, 입력 변환 테스트와 컨트롤러 동작 테스트를 분리할 수 있다. 다만 현재 요구사항만 보면 다소 큰 구조 변경이므로 장기 개선안에 가깝다.

## 리팩토링 우선순위 요약

1. `pushButton()`의 책임별 private helper 추출: 동작 보존 리팩토링으로 효과가 가장 크고 위험이 낮다.
2. 즐겨찾기 도메인 로직 분리: `toggle`과 `next` 정책은 컨트롤러보다 독립 도메인 객체에 가깝다.
3. 숫자 키 판별/변환 중복 제거: `switch` 길이를 줄이고 입력 정책을 명시화한다.
4. Magic Number 상수화: 채널 입력 정책을 이름 있는 상수로 드러낸다.
5. 전략 패턴, 테이블 기반 디스패치, `std::variant` 적용 검토: 키 종류와 상태 전이가 늘어날 때 OCP 개선 효과가 커진다.

## 개선 방향 요약

현재 `pushButton()`는 작은 기능 집합에서는 동작을 파악할 수 있지만, 리모컨 키와 채널 정책이 늘어날수록 변경 비용이 급격히 커지는 구조다. 1차로는 private helper 추출과 상수화로 SRP와 가독성을 개선하고, 2차로 즐겨찾기 로직을 별도 도메인 객체로 분리하는 것이 적절하다. 이후 키 확장이 빈번해지는 시점에 테이블 기반 디스패치나 전략 패턴을 적용하면 OCP 위반을 줄일 수 있다. `std::variant`는 물리 키 입력과 도메인 이벤트를 분리하고 싶을 때 유용하지만, 현재 규모에서는 즉시 도입보다 장기 선택지로 보는 것이 합리적이다.
