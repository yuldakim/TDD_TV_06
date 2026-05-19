# Defect Report

## 1. 문서 목적

본 문서는 `TDD_TV_06` 프로젝트의 결함을 일관된 기준으로 분류, 보고, 수정, 검증하기 위한 결함 관리 기준서이다. 결함은 `docs/defect_list.md`에 개별 항목으로 추적하고, 본 문서는 결함 등록 및 품질 메트릭 수집의 운영 기준으로 사용한다.

현재 기준선은 2026-05-19 검증 결과인 전체 35개 테스트 통과 상태이다. 신규 결함은 이 기준선 대비 재현 가능한 실패, 요구사항 미충족, 테스트 공백, 코드 품질 리스크, 빌드/환경 문제를 포함한다.

## 2. 결함 분류 체계

### Severity 정의

| Severity | 정의 | 처리 기준 |
| --- | --- | --- |
| Critical | 핵심 기능이 동작하지 않거나 테스트/빌드가 전면 중단되는 결함 | 즉시 수정, 릴리스 차단 |
| Major | 주요 요구사항 위반, 사용자 시나리오 실패, 회귀 가능성이 높은 결함 | 우선 수정, 릴리스 전 해결 |
| Minor | 제한된 조건의 오동작, 낮은 영향도의 품질 저하, 보완 테스트 필요 항목 | 계획 수정, 스프린트 내 추적 |
| Info | 제품 동작을 직접 깨지 않는 문서화, 관찰, 개선 제안 | 백로그 또는 참고 항목으로 관리 |

### ItemType 정의

| ItemType | 범위 | 예시 |
| --- | --- | --- |
| Requirement | 요구사항 정의, 해석, 누락, 충돌 | 채널 검색/업다운 요구사항의 진입 키 미정 |
| Functional | 제품 코드의 기능 동작 | 숫자 입력, `KEY_OK`, `KEY_MENU`, 즐겨찾기 토글, 순환 이동 |
| Test | 테스트 코드, 테스트 데이터, 승인 파일, 검증 공백 | 경계값 누락, 실패 assertion 부정확, approval 산출물 차이 |
| CodeQuality | 설계, 유지보수성, 예외 정책, 코드 스멜 | `pushButton()` 책임 과다, magic number, `stoi()` 예외 정책 |
| BuildInfra | 빌드, CI, 커버리지, 개발 환경 | CMake 설정, Google Test 실행, `gcov/lcov` 리포트 생성 실패 |

### Severity x ItemType 매트릭스

| Severity \ ItemType | Requirement | Functional | Test | CodeQuality | BuildInfra |
| --- | --- | --- | --- | --- | --- |
| Critical | 릴리스 필수 요구사항이 누락되어 전체 기능 판정이 불가능함 | 채널 변경 또는 즐겨찾기 핵심 흐름이 대부분 실패함 | 전체 테스트 실행이 불가능하거나 신뢰할 수 없음 | 런타임 크래시, 데이터 손상, 치명적 예외 전파가 일반 입력에서 발생함 | 빌드 또는 `ctest`가 전면 실패함 |
| Major | 명시 요구사항 중 주요 기능이 미구현 또는 상충됨 | P0/P1 사용자 시나리오가 실패하거나 회귀됨 | 핵심 기능의 실패를 테스트가 잡지 못함 | 변경 시 회귀 위험이 큰 구조적 문제 또는 예외 정책 부재 | 특정 타깃, 커버리지 수집, CI 단계가 반복 실패함 |
| Minor | 경계 조건 또는 보류 요구사항의 상세 정책이 불명확함 | 제한된 경계값에서 동작이 불명확하거나 낮은 영향도의 오동작 발생 | 경계값, 예외, fixture 구조 보완 필요 | 중복, 가독성 저하, magic number 등 유지보수성 문제 | 로컬 환경 의존, 리포트 경로, 실행 스크립트 보완 필요 |
| Info | 요구사항 추적성 향상을 위한 설명 보강 | 현재 동작 관찰 또는 향후 개선 제안 | 테스트 이름, 문서, 승인 산출물 정리 제안 | 리팩토링 후보, 네이밍 개선, 주석 보완 | 도구 설치 안내, 운영 문서 보강 |

## 3. 결함 보고서 템플릿

새 결함은 아래 템플릿을 사용해 `docs/defect_list.md` 또는 GitHub Issue에 등록한다.

```markdown
### DEFECT-XXX: 결함 제목

- 상태: Open / In Progress / Fixed / Closed / Won't Fix
- Severity: Critical / Major / Minor / Info
- ItemType: Requirement / Functional / Test / CodeQuality / BuildInfra
- 발견 단계: Requirements / Design / Unit Test / Integration Test / Regression / CI / Review
- 발견 일자: YYYY-MM-DD
- 담당자:
- 관련 요구사항:
- 관련 테스트:
- 대상 파일:

#### 재현 절차

1. 사전 조건을 작성한다.
2. 실행 입력 또는 명령을 작성한다.
3. 관찰된 실패를 작성한다.

#### 기대 결과

기대 동작, 통과해야 하는 테스트, 또는 충족해야 하는 요구사항을 작성한다.

#### 실제 결과

실제 동작, 실패 로그, assertion 차이, approval diff, 빌드 오류를 작성한다.

#### 원인 분석

제품 코드, 테스트 코드, 요구사항, 빌드 환경 중 어느 영역에서 문제가 발생했는지 분석한다.

#### 수정 방안

최소 수정 방향과 영향 범위를 작성한다. 동작 변경과 리팩토링은 가능한 한 분리한다.

#### 검증 결과

- 실행 명령:
- 테스트 결과:
- 커버리지 영향:
- 회귀 확인:
- 종료 기준 충족 여부:
```

## 4. 품질 메트릭 수집 계획

### 수집 대상 메트릭

| 메트릭 | 목적 | 수집 방법 | 기준 또는 목표 |
| --- | --- | --- | --- |
| 테스트 통과율 | 현재 변경이 기존 기능을 깨지 않았는지 확인 | `ctest --test-dir build --output-on-failure` 결과에서 통과/전체 테스트 수 집계 | 릴리스 전 100% |
| 라인 커버리지 | 제품 코드 실행 경로의 기본 누락 확인 | C++ `gcov/lcov`, Java JaCoCo, Python `pytest-cov` | `TVController.cpp` 95% 이상, 전체 90% 이상 |
| 브랜치 커버리지 | 조건 분기와 경계값 검증 수준 확인 | C++ `lcov --rc branch_coverage=1` 또는 `gcovr`, Java JaCoCo branch, Python `pytest-cov --cov-branch` | 핵심 컨트롤러 분기 90% 이상 |
| 단계별 결함 발견율 | 결함이 어느 단계에서 주로 발견되는지 추적 | 결함별 발견 단계 태그 집계 | 후반 단계 결함 비율 감소 |
| 결함 종료율 | 수정 처리 속도와 잔여 리스크 확인 | 기간별 Open/Closed 결함 수 집계 | Critical/Major는 릴리스 전 0건 |
| 재오픈율 | 수정 품질과 회귀 위험 확인 | Closed 후 재오픈된 결함 비율 집계 | 지속 감소 |

### 단계별 결함 발견율

단계별 결함 발견율은 다음 기준으로 집계한다.

```text
단계별 결함 발견율 = 해당 단계에서 발견된 결함 수 / 전체 발견 결함 수 * 100
```

권장 단계는 다음과 같다.

| 단계 | 설명 | 주요 산출물 |
| --- | --- | --- |
| Requirements | 요구사항 분석 중 발견 | `docs/requirements_analysis.md`, pending requirement |
| Design | 설계/코드 리뷰 중 발견 | `docs/code_quality_report.md`, `docs/review_report.md` |
| Unit Test | 단위 테스트 작성 또는 실행 중 발견 | Google Test, approval output |
| Regression | 기존 통과 테스트 재실행 중 발견 | `ctest`, golden master |
| CI | 자동 빌드/테스트/커버리지 중 발견 | GitHub Actions, coverage report |

초기에는 결함 수가 적을 수 있으므로 절대 건수와 비율을 함께 기록한다. 결함이 0건인 기간도 테스트 통과율과 커버리지 수치와 함께 기록해 기준선 유지 여부를 확인한다.

### C++ 수집 절차: gcov/lcov

현재 프로젝트는 C++17, CMake, Google Test를 기준으로 한다. Debug 빌드에서 coverage 옵션을 활성화한 뒤 테스트를 실행한다.

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

`lcov`가 설치된 환경에서는 제품 코드 중심 리포트를 생성한다.

```powershell
lcov --capture --directory build --output-file build/coverage.info
lcov --remove build/coverage.info "*/_deps/*" "*/test/*" "*/build/*" --output-file build/coverage.filtered.info
genhtml build/coverage.filtered.info --output-directory build/coverage-html
```

Windows에서 `lcov` 사용이 어렵다면 WSL, MSYS2, Linux CI, 또는 `gcovr`를 사용한다.

```powershell
gcovr -r . --filter "src/" --filter "include/" --exclude "build/" --html --html-details -o build/coverage.html
gcovr -r . --filter "src/" --filter "include/" --exclude "build/" --txt
```

### Java 수집 절차: JaCoCo

Java 모듈이 추가되는 경우 JaCoCo를 사용해 테스트와 커버리지를 함께 수집한다.

```powershell
./gradlew test jacocoTestReport
```

권장 산출물은 다음과 같다.

| 산출물 | 용도 |
| --- | --- |
| `build/reports/tests/test/index.html` | 테스트 통과율 확인 |
| `build/reports/jacoco/test/html/index.html` | 라인/브랜치 커버리지 확인 |
| `build/reports/jacoco/test/jacocoTestReport.xml` | CI 또는 PR 코멘트 연동 |

### Python 수집 절차: pytest-cov

Python 도구나 보조 스크립트가 추가되는 경우 `pytest-cov`를 사용한다.

```powershell
python -m pytest --cov=. --cov-branch --cov-report=term-missing --cov-report=xml
```

권장 산출물은 다음과 같다.

| 산출물 | 용도 |
| --- | --- |
| 터미널 `term-missing` 출력 | 미커버 라인 즉시 확인 |
| `coverage.xml` | CI 또는 커버리지 서비스 연동 |
| HTML 리포트 | 상세 분석 필요 시 사용 |

## 5. 결함 운영 흐름

1. 결함 발견 시 템플릿으로 재현 조건과 실제 결과를 먼저 기록한다.
2. Severity와 ItemType을 지정하고, 관련 요구사항/테스트/파일을 연결한다.
3. Critical/Major는 수정 전 실패 테스트 또는 재현 명령을 먼저 고정한다.
4. 수정은 최소 범위로 적용하고, 동작 변경과 리팩토링을 분리한다.
5. `cmake --build build`와 `ctest --test-dir build --output-on-failure`를 실행한다.
6. 영향 범위가 큰 경우 `gcov/lcov` 또는 `gcovr`로 커버리지 변화를 확인한다.
7. 검증 결과를 결함 항목에 기록하고, 재현 불가 또는 정책 결정 필요 항목은 상태와 근거를 명확히 남긴다.

## 6. GitHub Issues 연동 워크플로우

GitHub Issues를 사용하는 경우 결함 등록과 문서 추적을 다음 방식으로 연결한다.

### Issue 라벨

| 라벨 | 값 |
| --- | --- |
| `severity` | `severity:critical`, `severity:major`, `severity:minor`, `severity:info` |
| `type` | `type:requirement`, `type:functional`, `type:test`, `type:code-quality`, `type:build-infra` |
| `stage` | `stage:requirements`, `stage:design`, `stage:unit-test`, `stage:regression`, `stage:ci` |
| `status` | `status:open`, `status:fixed`, `status:blocked`, `status:needs-decision` |

### Issue 본문

Issue 본문은 본 문서의 결함 보고서 템플릿을 그대로 사용한다. 제목은 다음 형식을 권장한다.

```text
[Major][Functional] KEY_OK 단독 입력 시 예외 발생
```

### 브랜치 및 PR 흐름

| 단계 | 작업 |
| --- | --- |
| Issue 등록 | 재현 절차, 기대/실제 결과, Severity, ItemType 입력 |
| 브랜치 생성 | `fix/DEFECT-XXX-short-title` 형식 사용 |
| 테스트 고정 | 실패 테스트 또는 재현 가능한 회귀 테스트 추가 |
| 수정 | 최소 변경으로 결함 수정 |
| 검증 | 빌드, 테스트, 필요 시 커버리지 리포트 첨부 |
| PR 생성 | `Fixes #이슈번호`를 본문에 포함 |
| 종료 | CI 통과와 리뷰 승인 후 Issue 자동 종료 |

### 자동화 권장 항목

- PR마다 `cmake --build build`와 `ctest --test-dir build --output-on-failure` 실행
- Critical/Major 결함 PR에는 재현 테스트 추가 여부 체크리스트 포함
- 커버리지 리포트를 PR artifact로 업로드
- Issue 라벨별 Open 결함 수를 주간 품질 리포트에 집계
- `docs/defect_list.md`의 요약과 GitHub Issue 상태를 주기적으로 동기화

## 7. 현재 기준선

| 항목 | 현재 상태 |
| --- | --- |
| 검증 일자 | 2026-05-19 |
| 테스트 결과 | 35개 테스트 모두 통과 |
| Open 결함 | 없음 |
| 주요 미구현 요구사항 | 채널 검색, 채널 업/다운, 검색 결과 기반 업/다운 |
| 주요 관리 포인트 | 미구현 요구사항은 Requirement/Major 또는 보류 테스트로 추적 |

