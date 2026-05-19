# TDD TV 채널 컨트롤러 QA 종합 리뷰 보고서

## 1. 요구사항 충족도

`README.md`의 기능 요구사항을 기준으로 현재 구현 및 테스트를 매핑했다.

| README 요구사항 | 구현 상태 | 테스트 상태 | 평가 |
|---|---|---|---|
| 숫자 버튼으로 채널 변경: `1` + `확인` 입력 시 1번 채널로 변경 | `TVController::pushButton()`에서 숫자 입력을 `processingCH`에 저장하고 `KEY_OK`에서 `setCH()` 호출 | `TVControllerTest.Press1AndConfirmShouldChangeToChannel1` 존재 | 충족 |
| 숫자 버튼으로 채널 변경: `1`, `2` 입력 시 12번 채널로 변경 | 숫자 2개 입력 시 자동으로 `setCH()` 호출 | `TVControllerTest.PressMultipleDigitsAndConfirmShouldChangeChannel` 존재 | 충족 |
| `1`, `2`, `3`, `4` 연속 입력 시 12번, 34번으로 순차 변경 | 2자리 입력마다 `processingCH`를 초기화하는 방식으로 구현 | `TVControllerTest.S1_3_ContinuousInput` 존재 | 대체로 충족. 단, 호출 순서 검증은 없음 |
| `4`, `5`, `6` 입력 후 숫자와 확인 외 버튼을 누르면 마지막 `6` 무효화 | `KEY_MENU`에서 `processingCH` 초기화 | `TVControllerTest.S1_4_InvalidateOnlyLastDigitAfterAutoChange` 존재 | 부분 충족. "그 외의 버튼" 전체가 아니라 `MENU`만 검증 |
| `0`, `7` 입력 시 7번 채널로 변경 | 2자리 입력 시 `stoi()` 후 `to_string()`으로 앞자리 0 제거 | `TVControllerTest.S1_5_LeadingZeroShouldBeRemoved` 존재 | 충족 |
| 선호 채널 추가 버튼: 현재 채널이 선호 채널이 아니면 추가 | `KEY_FAV`에서 현재 채널을 `favorites`에 추가 | `FavoriteChannelTest.S2_1_AddCurrentChannelToFavorites` 존재 | 충족 |
| 선호 채널 추가 버튼: 이미 선호 채널이면 삭제 | `KEY_FAV`에서 기존 항목 발견 시 삭제 | `FavoriteChannelTest.S2_2_ToggleFavoriteRemoveIfExisted` 존재 | 충족 |
| 다음 선호 채널: 현재 채널보다 큰 값 중 가장 작은 선호 채널로 이동 | `std::upper_bound`로 다음 선호 채널 선택 | `FavoriteChannelTest.S3_1_NextFavoriteChannel_Normal` 존재 | 충족 |
| 다음 선호 채널: 마지막 선호 채널 이후에는 첫 선호 채널로 로테이션 | `upper_bound` 실패 시 `favorites[0]`으로 이동 | `FavoriteChannelTest.S3_3_NextFavoriteChannel_WrapAround` 존재 | 충족 |
| 채널 검색 버튼: 모든 채널을 검색하여 저장 | `remoteKey`에 채널 검색 버튼이 없고 `TVController` 구현 없음 | 관련 테스트 없음 | 미충족 |
| 업/다운 버튼: 검색 결과가 없으면 현재 채널 기준 +1/-1, 0~99 wrap-around | `remoteKey`에 업/다운 버튼이 없고 구현 없음 | 관련 테스트 없음 | 미충족 |
| 업/다운 버튼: 저장된 검색 결과가 있으면 다음/이전 검색 채널로 이동 | 검색 결과 저장소 및 업/다운 동작 구현 없음 | 관련 테스트 없음 | 미충족 |
| Tuner는 외부 제공으로 가정하고 Fake 또는 Mock 사용 | `Tuner` 추상 클래스, mock/fake 테스트 구현 | `MockTunerForController`, `FakeTunerForFav`, `FakeTunerForTunerTest` 존재 | 충족 |

요약하면 숫자 입력과 선호 채널 기능은 주요 요구사항이 구현 및 테스트되어 있다. 반면 README의 후반부 요구사항인 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운 기능은 현재 키 정의 단계부터 빠져 있어 요구사항 충족도가 낮다.

## 2. AAA 패턴 준수도

| 테스트 파일 | AAA 준수도 | 근거 | 개선 방향 |
|---|---|---|---|
| `test/TVControllerTest.cpp` | 보통 | `MockTuner` 생성과 `EXPECT_CALL`은 Arrange, `pushButton()` 호출은 Act, mock 검증은 Assert 역할을 한다. 다만 approval 파일 기록이 Assert 이후에 섞여 있다. | Given/When/Then 주석 또는 fixture helper로 Arrange, Act, Assert를 더 명확히 분리 |
| `test/FavoriteChannelTest.cpp` | 보통 | fixture로 Arrange를 일부 분리했고 `EXPECT_EQ`로 명시 검증한다. 그러나 `verifyApproval()` 파일 기록이 테스트 본문마다 추가되어 핵심 assertion과 보조 검증이 섞인다. | approval 검증을 별도 integration/characterization test로 분리 |
| `test/TunerTest.cpp` | 보통 | 입력 배열 준비, 반복 실행, assertion 구조는 명확하다. 다만 Tuner는 외부 제공 가정인데 테스트 내 fake 구현 검증이 제품 요구사항 검증처럼 보일 수 있다. | 외부 Tuner 계약 검증 목적이면 별도 contract test로 명명 |

대표적으로 현재 테스트는 AAA 흐름을 어느 정도 따르지만, approval 파일 생성 로직이 unit test 내부에 함께 들어가 테스트 의도를 흐린다.

```cpp
MockTunerForController mockTuner;
EXPECT_CALL(mockTuner, setCH("1")).Times(1);

TVController controller(&mockTuner);
controller.pushButton(remoteKey::KEY_1);
controller.pushButton(remoteKey::KEY_OK);

verifyTVControllerApproval("S1_1_Press1AndConfirmShouldChangeToChannel1",
                           "1");
```

권장 형태는 핵심 검증과 보조 산출물 생성을 분리하는 것이다.

```cpp
// Arrange
MockTunerForController mockTuner;
EXPECT_CALL(mockTuner, setCH("1")).Times(1);
TVController controller(&mockTuner);

// Act
controller.pushButton(remoteKey::KEY_1);
controller.pushButton(remoteKey::KEY_OK);

// Assert
// Mock expectation verifies that channel 1 was selected.
```

## 3. 경계값 테스트 존재 여부

| 경계값/예외 조건 | 테스트 존재 여부 | 현재 테스트 | 평가 |
|---|---|---|---|
| 최소 채널 0 | 있음 | `TunerTest.initChannel`, `TunerTest.testSetChForValidChannel` | Tuner fake 기준으로 검증됨 |
| 최대 채널 99 | 있음 | `TunerTest.testSetChForValidChannel`, `testSeekCh10timesAfterSetCH` | Tuner fake 기준으로 검증됨 |
| 범위 초과 100 이상 | 있음 | `TunerTest.testSetChForInvalidChannel` | `100`, `9999` 검증 |
| 음수 채널 | 있음 | `TunerTest.testSetChForInvalidChannel` | `-12` 검증 |
| 앞자리 0 입력 | 있음 | `TVControllerTest.S1_5_LeadingZeroShouldBeRemoved` | `07` -> `7` 검증 |
| `00` 입력 | 없음 | 없음 | 0번 채널 처리 명확성 보강 필요 |
| `09` 입력 | 없음 | 없음 | 앞자리 0 제거의 일반화 검증 필요 |
| `OK` 단독 입력 | 없음 | 없음 | 빈 입력에서 `setCH()`가 호출되지 않아야 함 |
| 숫자 1개 입력 후 `MENU` 취소 | 있음 | `S1_4_InvalidateOnlyLastDigitAfterAutoChange` 일부 | `6` 취소 케이스만 검증 |
| 숫자 1개 입력 후 `FAV`, `NEXT_FAV` 등 다른 버튼 | 없음 | 없음 | README의 "그 외의 버튼" 요구사항 대비 부족 |
| 선호 채널 목록 비어 있음 | 있음 | `FavoriteChannelTest.S3_4_NextFavoriteChannel_WhenListIsEmpty` | 충족 |
| 선호 채널 목록에서 wrap-around | 있음 | `FavoriteChannelTest.S3_3_NextFavoriteChannel_WrapAround` | 충족 |
| 채널 업 99 -> 0 | 없음 | 없음 | 기능 자체 미구현 |
| 채널 다운 0 -> 99 | 없음 | 없음 | 기능 자체 미구현 |
| 검색 결과 기반 업/다운 wrap-around | 없음 | 없음 | 기능 자체 미구현 |

경계값 테스트는 Tuner fake와 선호 채널 일부에는 존재한다. 그러나 `TVController` 관점의 핵심 경계값, 특히 `00`, `09`, 빈 입력 `OK`, 업/다운 wrap-around는 추가가 필요하다.

## 4. 개선 권고사항

### Critical

| 항목 | 문제 | 권고 |
|---|---|---|
| README 요구사항 미구현 | 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운 기능이 구현되어 있지 않다. `remoteKey`에도 관련 키가 없다. | `KEY_SEARCH`, `KEY_CH_UP`, `KEY_CH_DOWN` 등을 정의하고 요구사항별 테스트를 먼저 추가한 뒤 구현 |
| 요구사항 추적성 부족 | README에는 전체 기능이 정의되어 있지만 테스트는 일부 기능에 집중되어 있다. | README 요구사항별 테스트 케이스 매트릭스를 유지하고 미구현 항목을 명시 |

### Major

| 항목 | 문제 | 권고 |
|---|---|---|
| `TVController` 책임 과다 | 입력 버퍼, 채널 전환, 선호 채널 저장/정렬/탐색을 한 클래스가 모두 담당한다. | `ChannelInputBuffer`, `FavoriteChannels`, `ChannelSearchResult` 등으로 책임 분리 |
| approval test와 unit test 혼합 | 테스트가 명시적 assertion과 파일 기반 approval 산출물을 동시에 수행한다. | unit test는 assertion 중심으로 유지하고 approval test는 별도 target 또는 별도 fixture로 분리 |
| 호출 순서 검증 부족 | 연속 입력 테스트에서 `setCH("12")`, `setCH("34")`의 순서를 보장하지 않는다. | GoogleMock `InSequence`를 사용하여 순서 검증 |
| raw pointer 의존성 | `TVController`가 `Tuner*`를 저장해 null pointer 가능성이 있다. | 생성자에서 `Tuner&`를 받거나 null check를 추가 |
| 인터페이스 분리 부족 | `TVController`는 `seekCH()`를 사용하지 않지만 `Tuner` 전체에 의존한다. | `IChannelReader`, `IChannelWriter`, `IChannelScanner`로 역할별 인터페이스 분리 |

### Minor

| 항목 | 문제 | 권고 |
|---|---|---|
| 테스트 이름 일관성 | 일부 테스트는 요구사항 중심 이름, 일부는 `S1_3` 형식이다. | `When...Should...` 또는 요구사항 ID 기반으로 네이밍 규칙 통일 |
| 상대 경로 기반 파일 IO | approval 파일 경로가 `../test/...`로 고정되어 실행 위치에 민감하다. | CMake working directory 정리 또는 테스트 리소스 경로 상수화 |
| 테스트 내 동적 할당 | `FavoriteChannelTest`에서 `TVController*`를 `new/delete`로 관리한다. | 값 멤버 또는 `std::unique_ptr` 사용 |
| 제품 코드의 콘솔 출력 | `setTunerCh()`에서 `std::cout`을 호출한다. | unit test 노이즈를 줄이기 위해 제거하거나 logger 주입 |

## 5. 잘 구현된 부분

| 항목 | 긍정 평가 |
|---|---|
| 추상화 기반 테스트 | `Tuner`를 추상 클래스로 정의하고 `MockTunerForController`, `FakeTunerForFav`를 사용해 컨트롤러를 하드웨어 구현과 분리했다. |
| 숫자 입력 핵심 시나리오 | 한 자리 입력, 두 자리 자동 확정, 연속 네 자리 입력, 앞자리 0 제거 등 주요 입력 흐름이 테스트되어 있다. |
| 선호 채널 기능 | 추가, 삭제 토글, 정렬 유지, 다음 선호 채널 이동, wrap-around, 빈 목록 처리가 테스트되어 있다. |
| 표준 라이브러리 활용 | `std::find`, `std::sort`, `std::upper_bound`를 사용해 선호 채널 탐색 로직을 간결하게 구현했다. |
| CMake 테스트 타깃 분리 | `TunerTest`, `TVControllerTest`, `FavoriteChannelTest`를 별도 실행 파일로 분리해 기능별 테스트 실행이 가능하다. |
| 커버리지 플래그 준비 | Debug 빌드에서 `--coverage` 옵션을 활성화해 정량 커버리지 분석 기반을 마련했다. |

## 6. 결론

현재 코드베이스는 TDD 실습의 초기 목표인 숫자 채널 입력과 선호 채널 기능에 대해 의미 있는 테스트와 구현을 갖추고 있다. 특히 `Tuner`를 mock/fake로 대체할 수 있게 만든 점은 테스트 가능성 측면에서 좋은 선택이다.

다만 README 기준 전체 요구사항 관점에서는 채널 검색 및 업/다운 기능이 아직 빠져 있어 완성도는 부분 충족 수준이다. 다음 단계에서는 누락된 버튼과 동작을 테스트로 먼저 정의하고, `TVController`의 책임을 입력 처리, 선호 채널 관리, 검색 결과 관리로 분리하는 리팩터링을 병행하는 것이 좋다.
