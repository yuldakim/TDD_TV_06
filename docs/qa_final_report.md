# Gilded Rose C++ QA Final Report

## 1. Executive Summary

본 QA 활동의 현재 기준선은 `2026-05-19` 검증 결과 기준으로 `36/36` 테스트 통과, 테스트 통과율 `100%`, 재현 가능한 Open 결함 `0건`이다. 핵심 구현 범위인 숫자 채널 입력, `OK` 확정, `MENU` 취소, 선호 채널 추가/삭제/정렬/순환은 단위 테스트와 Golden Master 테스트로 회귀 보호가 형성되어 있다.

다만 README/요구사항 관점의 전체 완성도는 부분 충족이다. 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운은 `remoteKey` 진입점부터 미구현이며, 커버리지 목표도 일부 미달한다. 특히 `src/remoteKey.cpp`의 `to_string(remoteKey)`는 현재 깨끗한 임시 빌드 기준 실행 커버리지가 `0%`로 측정되어 테스트 계획의 `100%` 목표와 차이가 크다.

## 2. 테스트 완료율 및 커버리지

### 테스트 완료율

| 항목 | 목표 | 현재 결과 | 판정 |
| --- | ---: | ---: | --- |
| 빌드 | 성공 | 성공 | 충족 |
| Google Test/CTest | 100% 통과 | `36/36`, `100%` 통과 | 충족 |
| P0 핵심 채널 변경 | 전부 통과 | 한 자리, 두 자리, 연속 입력, 취소, 경계값 통과 | 충족 |
| P0 선호 채널 | 전부 통과 | 추가, 삭제, 정렬, 다음 이동, 순환, 빈 목록 통과 | 충족 |
| P2 미구현 요구사항 | 보류 추적 | 채널 검색/업/다운 미구현 | 보류 |

### gcov 측정 결과

측정은 기존 `build`의 누적 `.gcda` 영향을 피하기 위해 임시 Debug 빌드에서 수행했다.

```powershell
cmake -S . -B $env:TEMP\TDD_TV_06_qa_coverage -DCMAKE_BUILD_TYPE=Debug
cmake --build $env:TEMP\TDD_TV_06_qa_coverage
ctest --test-dir $env:TEMP\TDD_TV_06_qa_coverage --output-on-failure
gcov -b -c <product gcda>
```

| 대상 | 목표 | 현재 gcov 결과 | Gap | 판정 |
| --- | ---: | ---: | ---: | --- |
| 전체 제품 코드 라인 | 90% 이상 | 약 `70.5%` (`src/TVController.cpp`, `src/FavoriteChannels.cpp`, `src/remoteKey.cpp` 포함) | `-19.5%p` | 미달 |
| `src/TVController.cpp` 라인 | 95% 이상 | 약 `92.7%` 추정 통합 기준, Golden Master 단일 타깃 `90.24%` | `-2.3%p` | 근접 미달 |
| `src/TVController.cpp` branch executed | 90% 이상 | Golden Master 단일 타깃 `90.91%` | `+0.91%p` | 충족 |
| `src/TVController.cpp` taken at least once | 보조 지표 | Golden Master 단일 타깃 `59.09%` | - | 개선 필요 |
| `src/FavoriteChannels.cpp` 라인 | 90% 이상 | `93.75%` | `+3.75%p` | 충족 |
| `src/FavoriteChannels.cpp` branch executed | 90% 이상 | `100%` | `+10%p` | 충족 |
| `src/remoteKey.cpp` 라인 | 100% | `0%` | `-100%p` | 미달 |

커버리지 미달의 주된 원인은 `remoteKey::to_string()` 직접 테스트 부재, `KEY_8`/알 수 없는 enum/default 경로 미검증, 예외 경로와 defensive branch의 미실행이다. 제품 기능 리스크는 낮지만, 테스트 계획 대비 커버리지 목표는 아직 완료 상태가 아니다.

## 3. 결함 패턴 분석

### 등록 결함 현황

| 상태 | 건수 | 설명 |
| --- | ---: | --- |
| Open | 0 | 현재 재현 가능한 실패 없음 |
| Closed/기준선 통과 | 36 tests | 전체 테스트 통과 |
| 보류 요구사항 | 3 | 채널 검색, 채널 업, 채널 다운/검색 결과 기반 이동 |

### ItemType별 잔여 리스크

| ItemType | 주요 패턴 | 대표 사례 | QA 판단 |
| --- | --- | --- | --- |
| Requirement | 구현 진입점 누락 | `KEY_SEARCH`, `KEY_CH_UP`, `KEY_CH_DOWN` 부재 | Major. 전체 요구사항 완료율을 제한한다. |
| Functional | 현재 구현 범위 내 실패 없음 | 숫자 입력/선호 채널 통과 | Closed. 회귀 테스트 유지가 중요하다. |
| Test | 커버리지 공백 | `remoteKey::to_string()`, `KEY_8`, default enum, 일부 예외 정책 | Major. 목표 커버리지 미달의 직접 원인이다. |
| CodeQuality | 유지보수 리스크 | `Tuner*` null 가능성, `std::stoi()` 예외 정책, 콘솔 출력 결합 | Major/Minor. 기능 확장 전 정리 필요하다. |
| BuildInfra | 측정 자동화 미흡 | `gcovr` 미설치, `lcov`/HTML 리포트 없음 | Minor. CI 품질 게이트로 보완 필요하다. |

### 심각도별 패턴

| Severity | 현재 판단 | 분석 |
| --- | ---: | --- |
| Critical | 0 | 빌드/테스트 전면 실패나 핵심 기능 중단은 없다. |
| Major | 5 | 미구현 요구사항 3건, 커버리지 목표 미달, 확장 전 구조 리스크가 주요 항목이다. |
| Minor | 4 | 커버리지 도구 자동화, 테스트 네이밍/추적성, 콘솔 출력, 일부 예외 정책 문서화가 해당된다. |
| Info | 2 | Golden Master 산출물, 결함 템플릿 등 운영 개선 항목이다. |

## 4. 9단계 QA 활동 평가

| 단계 | 효과 | 평가 |
| --- | --- | --- |
| 1. 요구사항 분석 | 높음 | 구현/미구현 범위를 조기에 분리했고 P2 보류 요구사항을 명시했다. |
| 2. 코드 구조 분석 | 높음 | `TVController`, `Tuner`, `remoteKey`, `FavoriteChannels`의 책임과 의존성을 파악해 테스트 전략을 세웠다. |
| 3. Golden Master 작성 | 높음 | 레거시 동작을 승인 파일로 고정해 리팩토링 안정성을 높였다. |
| 4. 테스트 계획 수립 | 높음 | P0/P1/P2, 경계값, 예외, 커버리지 목표가 문서화됐다. |
| 5. 단위 테스트 보강 | 높음 | 현재 구현 범위의 핵심 사용자 시나리오가 `36`개 테스트로 보호된다. |
| 6. 리팩토링 | 중상 | `FavoriteChannels` 분리와 helper 추출로 이전 Long Method/SRP 리스크가 줄었다. |
| 7. 결함 관리 체계화 | 중상 | Severity/ItemType 템플릿과 기준선이 생겼다. 다만 실제 defect ticket 누적 데이터는 아직 적다. |
| 8. 커버리지 측정 | 개선 필요 | `--coverage` 기반은 준비됐지만 `gcovr/lcov` 자동 리포트와 목표 게이트가 없다. |
| 9. CI/품질 게이트 | 개선 필요 | 로컬 검증은 가능하나 PR마다 테스트/커버리지/결함 요약을 자동화하는 단계가 필요하다. |

가장 효과적이었던 단계는 요구사항 분석, Golden Master, 단위 테스트 보강이다. 개선이 필요한 단계는 커버리지 자동화와 CI 품질 게이트이며, 다음 반복에서는 `remoteKey` 테스트와 P2 요구사항 테스트를 먼저 활성화하는 것이 좋다.

## 5. 다음 레거시 프로젝트 Best Practice 5가지

1. 요구사항을 먼저 `충족/부분충족/미구현/정책미정`으로 나누고, 미구현 항목은 실패 테스트가 아니라 보류 테스트 또는 명시적 TODO로 추적한다.
2. 레거시 동작 변경 전 Golden Master를 만들어 현재 동작을 고정하고, 이후 단위 테스트로 의도를 좁혀 간다.
3. Mock은 호출 계약 검증에, Fake는 상태 기반 도메인 검증에 사용해 테스트 더블의 목적을 분리한다.
4. 리팩토링은 커버리지와 승인 테스트가 있는 범위에서 작게 수행하고, 동작 변경과 구조 변경을 같은 커밋/PR에 섞지 않는다.
5. `ctest`, `gcov/lcov` 또는 `gcovr`, 결함 요약을 CI에 연결해 품질 기준을 사람의 기억이 아니라 자동 게이트로 둔다.

## 6. Cursor AI 활용 효과

### 정량 요약

| 항목 | 효과 |
| --- | --- |
| 테스트 수 | 기준선 `36`개 테스트 통과 상태 확보 |
| 조기 리스크 식별 | 미구현 요구사항 3건, 커버리지 미달 3개 영역, 코드품질 리스크 5개 이상 식별 |
| 문서화 산출물 | 요구사항 분석, 테스트 계획, 코드 품질 리포트, 결함 리포트, 최종 QA 리포트로 추적성 강화 |
| 커버리지 개선 방향 | `TVController.cpp`는 목표에 근접, `FavoriteChannels.cpp`는 목표 충족, `remoteKey.cpp` 공백을 명확히 드러냄 |
| 시간 단축 추정 | 수동 분석/문서화 대비 약 `30~50%` 단축. 단, 실제 작업 시간 로그가 없어 산출물 기반 추정치로 관리해야 한다. |

### 정성 요약

Cursor AI는 레거시 코드의 요구사항 역추적, 테스트 공백 탐지, 리팩토링 후보 정리, 결함 분류 체계 작성에 특히 효과적이었다. QA 리드 관점에서 가장 큰 장점은 “무엇이 통과했는가”뿐 아니라 “아직 검증되지 않은 것은 무엇인가”를 문서와 테스트 이름으로 빠르게 드러낸 점이다.

반면 커버리지 수치 해석은 도구 상태에 민감했다. 기존 `build`의 누적 `.gcda`는 실제 현재 코드 기준과 다를 수 있어 임시 클린 빌드가 필요했고, `gcovr` 미설치로 통합 리포트 자동화가 부족했다. 다음 프로젝트에서는 Cursor AI가 생성한 테스트/문서와 별도로 CI에서 커버리지 리포트를 재현 가능하게 생성하도록 초기에 세팅하는 것이 필요하다.

## 7. 최종 QA 판단

현재 프로젝트는 구현된 범위에 대해서는 릴리스 후보 수준의 회귀 보호를 갖췄다. 그러나 전체 요구사항 기준 릴리스 완료로 보기에는 채널 검색/업다운 기능 미구현과 `remoteKey` 커버리지 공백이 남아 있다.

다음 액션 우선순위는 다음과 같다.

1. `remoteKey::to_string()` 전체 enum 테스트를 추가해 `src/remoteKey.cpp` 라인 커버리지 `100%`를 달성한다.
2. `KEY_SEARCH`, `KEY_CH_UP`, `KEY_CH_DOWN` 요구사항을 Pending test에서 활성 테스트로 전환한다.
3. `TVController.cpp`의 `KEY_8`, default enum, 예외 정책 테스트를 보강해 라인 `95%+`, branch taken 지표를 개선한다.
4. `gcovr` 또는 `lcov`를 CI에 설치하고 PR 품질 게이트로 `ctest + coverage`를 고정한다.
