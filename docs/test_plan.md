# TVController 테스트 계획서

## 1. 목적

`TVController`의 채널 입력, 입력 취소, 즐겨찾기 등록/삭제/순환 동작이 요구사항대로 동작하는지 Google Test 기반 단위 테스트로 검증한다. 테스트는 C++17, Google Test/Google Mock, CMake 환경을 기준으로 하며, 가능하면 `gcov/lcov`로 라인/브랜치 커버리지를 측정한다.

## 2. 테스트 대상 및 범위

### 대상 코드

- `include/TVController.h`
- `src/TVController.cpp`
- 협력 객체: `Tuner`
- 보조 타입: `remoteKey`, `to_string(remoteKey)`

### 검증 범위

- 숫자 키 입력 버퍼링 및 채널 확정
- `KEY_OK`에 의한 한 자리 채널 확정
- 두 자리 입력 시 자동 채널 확정
- 앞자리 `0` 정규화
- `KEY_MENU`에 의한 미확정 입력 취소
- 즐겨찾기 추가, 삭제, 중복 방지, 오름차순 정렬
- `KEY_NEXT_FAV`의 다음 즐겨찾기 이동 및 순환
- 빈 즐겨찾기 목록에서 no-op 처리
- `Tuner::getCurrentCH()` 계약 위반 시 예외 전파 특성

### 범위 제외 또는 보류

다음 요구사항은 현재 `remoteKey`와 `TVController` 구현에 진입점이 없어 보류 테스트로 관리한다.

- 채널 검색 키
- 채널 업/다운 키
- 검색 결과 기반 채널 업/다운
- 검색 결과가 없을 때 `0`~`99` 순환 이동

해당 기능이 구현되면 `PendingRequirementTest` 또는 신규 fixture로 활성화한다.

## 3. 테스트 접근 방식

### TEST_F 기반 구조

현재 `FavoriteChannelTest`는 `TEST_F`를 사용하지만, `TVControllerTest`의 숫자 입력 테스트는 `TEST` 기반이다. 컨트롤러 테스트는 공통 fixture를 도입해 상태 초기화, mock/fake 튜너 준비, 공통 입력 helper를 재사용하는 형태를 우선한다.

권장 fixture:

- `TVControllerDigitInputTest`: 숫자 입력, OK, MENU, 자동 확정 검증
- `FavoriteChannelTest`: 즐겨찾기 추가/삭제/정렬/순환 검증
- `TVControllerExceptionalTest`: 비정상 `Tuner` 응답, `nullptr` 정책 등 특이 케이스 검증
- `RemoteKeyTest`: `remoteKey` 문자열 변환 계약 검증
- `TunerContractTest`: `Tuner` 테스트 더블의 채널 범위 계약 검증

테스트 더블 전략:

- 숫자 입력 검증은 `MockTuner`와 `EXPECT_CALL`을 사용해 `setCH()` 호출 인자와 횟수를 검증한다.
- 즐겨찾기 검증은 상태 관찰이 쉬운 `FakeTuner`를 사용해 현재 채널과 `getFavorites()` 결과를 검증한다.
- 예외 검증은 의도적으로 비숫자 문자열, 빈 문자열, 범위 밖 값을 반환하는 fake를 별도로 둔다.

## 4. 우선순위별 단위 테스트 범위

### P0: 핵심 채널 변경 동작

| ID | 테스트 | 검증 포인트 |
| --- | --- | --- |
| P0-01 | `KEY_1`, `KEY_OK` | `setCH("1")` 1회 호출 |
| P0-02 | `KEY_1`, `KEY_2` | `setCH("12")` 자동 호출 |
| P0-03 | `KEY_1`, `KEY_2`, `KEY_OK` | 자동 확정 후 `KEY_OK` 추가 호출 없음 |
| P0-04 | `KEY_1`, `KEY_2`, `KEY_3`, `KEY_4` | `12`, `34` 순서로 확정 |
| P0-05 | 빈 버퍼에서 `KEY_OK` | `setCH()` 호출 없음 |
| P0-06 | `KEY_6`, `KEY_MENU`, `KEY_OK` | 취소된 `6`은 확정되지 않음 |

### P0: 즐겨찾기 핵심 동작

| ID | 테스트 | 검증 포인트 |
| --- | --- | --- |
| P0-07 | 현재 채널 `10`에서 `KEY_FAV` | 즐겨찾기 `[10]` |
| P0-08 | 같은 채널에서 `KEY_FAV` 2회 | 추가 후 삭제, 최종 빈 목록 |
| P0-09 | `12`, `8`, `37`, `8`, `6` 토글 | 중복 없이 `[6, 12, 37]` 정렬 |
| P0-10 | 즐겨찾기 `[1, 4, 12, 56]`, 현재 `6` | 다음 즐겨찾기 `12`로 이동 |
| P0-11 | 즐겨찾기 `[1, 4, 12, 56]`, 현재 `56` | `1`로 순환 |
| P0-12 | 빈 즐겨찾기에서 `KEY_NEXT_FAV` | 채널 변경 없음 |

### P1: 경계값 및 정규화

| ID | 테스트 | 검증 포인트 |
| --- | --- | --- |
| P1-01 | `KEY_0`, `KEY_0` | `setCH("0")` 호출 |
| P1-02 | `KEY_0`, `KEY_7` | 앞자리 0 제거 후 `setCH("7")` |
| P1-03 | `KEY_9`, `KEY_9` | 최대 유효 채널 `99` 확정 |
| P1-04 | `KEY_0`, `KEY_OK` | 한 자리 `0` 확정 |
| P1-05 | `KEY_4`, `KEY_5`, `KEY_6`, `KEY_MENU`, `KEY_OK` | 이미 확정된 `45`는 유지, 미확정 `6`만 취소 |
| P1-06 | 즐겨찾기 1개에서 `KEY_NEXT_FAV` | 같은 채널로 유지 또는 동일 값 재설정 |

### P1: 협력 객체 계약

| ID | 테스트 | 검증 포인트 |
| --- | --- | --- |
| P1-07 | `to_string(KEY_0..KEY_9)` | `"0"`~`"9"` 반환 |
| P1-08 | `to_string(KEY_OK/MENU/FAV/NEXT_FAV)` | `"OK"`, `"MENU"`, `"FAV"`, `"NEXT_FAV"` 반환 |
| P1-09 | `Tuner::setCH("0")`, `"99"` | 경계값 허용 |
| P1-10 | `Tuner::setCH("-1")`, `"100"` | 범위 밖 값 거부 |

### P2: 미구현 요구사항 회귀 방지용 보류 테스트

| ID | 테스트 | 기대 동작 |
| --- | --- | --- |
| P2-01 | 채널 검색 키 입력 | 시청 가능 채널 목록 저장 |
| P2-02 | 검색 결과 없음, 채널 업 | `99 -> 0` 순환 |
| P2-03 | 검색 결과 없음, 채널 다운 | `0 -> 99` 순환 |
| P2-04 | 검색 결과 `[4, 6, 14]`, 현재 `6` | 업 `14`, 다운 `4` |
| P2-05 | 검색 결과 `[4, 6, 14]`, 현재 `15` | 업 `4`, 다운 `14` 순환 |

## 5. 경계값 케이스 목록

| 분류 | 입력/상태 | 기대 결과 |
| --- | --- | --- |
| 최소 채널 | `0`, `OK` 또는 `0`, `0` | 채널 `0`으로 설정 |
| 최대 채널 | `9`, `9` | 채널 `99`로 설정 |
| 앞자리 0 | `0`, `7` | 채널 `7`로 정규화 |
| 두 자리 자동 확정 직후 | `1`, `2`, `OK` | `12`만 1회 설정 |
| 연속 입력 경계 | `1`, `2`, `3`, `4` | `12`, `34` 두 번 설정 |
| 미확정 입력 취소 | `6`, `MENU`, `OK` | 채널 변경 없음 |
| 확정 후 취소 | `4`, `5`, `6`, `MENU`, `OK` | `45`만 설정 |
| 빈 버퍼 OK | `OK` | no-op |
| 즐겨찾기 빈 목록 | `KEY_NEXT_FAV` | no-op |
| 즐겨찾기 단일 원소 | `[7]`, 현재 `7`, `KEY_NEXT_FAV` | `7` 유지 |
| 즐겨찾기 순환 상단 | `[1, 4, 12, 56]`, 현재 `56` | `1`로 이동 |
| 즐겨찾기 중간값 | `[1, 4, 12, 56]`, 현재 `6` | `12`로 이동 |
| 즐겨찾기 현재값 미포함 | `[10, 20]`, 현재 `15` | `20`으로 이동 |
| 즐겨찾기 현재값보다 큰 값 없음 | `[10, 20]`, 현재 `21` | `10`으로 순환 |

## 6. 예외 및 특이 케이스 목록

| 분류 | 조건 | 현재 기대 결과 | 비고 |
| --- | --- | --- | --- |
| 비숫자 현재 채널 | `getCurrentCH()`가 `"ABC"` 반환 후 `KEY_FAV` | `std::invalid_argument` 전파 | 현재 구현은 예외를 잡지 않음 |
| 빈 현재 채널 | `getCurrentCH()`가 `""` 반환 후 `KEY_NEXT_FAV` | `std::invalid_argument` 전파 가능 | 즐겨찾기가 비어 있으면 `stoi()` 호출 전 종료 |
| 정수 범위 초과 문자열 | `getCurrentCH()`가 매우 큰 숫자 문자열 반환 | `std::out_of_range` 전파 가능 | `std::stoi` 계약 확인 |
| `nullptr` 튜너 | `TVController(nullptr)` 후 키 입력 | 역참조 실패 가능 | death test 또는 생성 방어 정책 결정 필요 |
| `Tuner::setCH()` 예외 | 튜너가 범위/형식 예외 발생 | 예외 전파 | 컨트롤러는 복구 로직 없음 |
| 알 수 없는 enum 값 | `static_cast<remoteKey>(999)` | `to_string()`은 `""`, `pushButton()`은 default 없음 | default 처리 정책 검토 필요 |
| 콘솔 출력 | `setTunerCh()`가 `std::cout` 출력 | 테스트 결과에는 영향 없어야 함 | 필요 시 stdout capture |

`nullptr` 튜너와 알 수 없는 enum 값은 제품 정책이 먼저 결정되어야 한다. 현재 구현을 기준으로는 방어 코드가 없으므로 "지원하지 않는 입력"으로 명시하거나, 생성자에서 `std::invalid_argument`를 던지도록 개선한 뒤 테스트를 고정하는 것이 바람직하다.

## 7. 커버리지 목표

### 목표

- 전체 라인 커버리지: 90% 이상
- `src/TVController.cpp` 라인 커버리지: 95% 이상
- `src/TVController.cpp` 브랜치 커버리지: 90% 이상
- `src/remoteKey.cpp` 라인 커버리지: 100%
- 테스트 실패 시 승인 테스트 산출물(`*.received.txt`) 차이를 함께 확인

### 우선 개선 대상

1. `pushButton()`의 모든 `case` 분기 커버
2. `KEY_OK`의 버퍼 있음/없음 양쪽 분기 커버
3. `KEY_FAV`의 추가/삭제 양쪽 분기 커버
4. `KEY_NEXT_FAV`의 빈 목록, 다음 원소 존재, wrap-around 분기 커버
5. 숫자 입력의 길이 1/2 경계와 자동 확정 후 버퍼 초기화 검증
6. `to_string(remoteKey)`의 모든 enum case 커버

## 8. gcov/lcov 측정 전략

현재 `CMakeLists.txt`는 `Debug` 빌드에서 `--coverage -O0` 옵션을 추가한다. GCC 또는 MinGW 계열 컴파일러를 사용하는 환경에서 아래 절차로 측정한다.

### 측정 절차

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

`lcov`가 설치되어 있다면 다음과 같이 제품 코드 중심 리포트를 생성한다.

```powershell
lcov --capture --directory build --output-file build/coverage.info
lcov --remove build/coverage.info "*/_deps/*" "*/test/*" "*/build/*" --output-file build/coverage.filtered.info
genhtml build/coverage.filtered.info --output-directory build/coverage-html
```

Windows에서 `lcov` 사용이 어렵다면 WSL, MSYS2, 또는 Linux CI에서 동일한 CMake 명령을 실행하는 방식을 권장한다. 대안으로 `gcovr`를 사용할 수 있다.

```powershell
gcovr -r . --filter "src/" --filter "include/" --exclude "build/" --html --html-details -o build/coverage.html
gcovr -r . --filter "src/" --filter "include/" --exclude "build/" --txt
```

### 개선 전략

- 첫 측정은 `TVController.cpp`, `remoteKey.cpp`만 필터링해 컨트롤러 로직의 공백을 확인한다.
- 미커버 라인이 `KEY_NEXT_FAV`, `KEY_FAV`의 특정 분기에 몰려 있으면 fixture 기반 상태 세팅 helper를 추가해 조합 테스트를 늘린다.
- 라인 커버리지는 높지만 브랜치 커버리지가 낮으면 `KEY_OK` 빈/비어있음, 즐겨찾기 empty/non-empty, `upper_bound()` hit/miss 케이스를 추가한다.
- approval test는 회귀 감지 보조 수단으로 유지하되, 핵심 판정은 `EXPECT_CALL`, `EXPECT_EQ`, `EXPECT_THROW` 같은 명시적 assertion으로 둔다.
- 미구현 기능은 커버리지 목표에서 제외하되, 구현 시 P2 보류 테스트를 활성화하고 커버리지 기준에 포함한다.

## 9. 테스트 완료 기준

- P0 테스트 전부 통과
- P1 테스트 중 구현 정책이 확정된 항목 전부 통과
- `ctest --test-dir build --output-on-failure` 통과
- `TVController.cpp` 라인 커버리지 95% 이상, 브랜치 커버리지 90% 이상
- 미구현 요구사항은 보류 테스트 또는 명시적 TODO로 추적
- 예외/특이 케이스 중 정책 미정 항목은 제품 요구사항 또는 설계 문서에 결정 사항 반영
