# Golden Master 회귀 테스트

## 기준 출력 파일 전략

- 기준 파일은 `test/golden/TVControllerGoldenMaster.approved.txt`에 보관하고 Git에 커밋한다.
- 테스트 실행 중 생성되는 실제 출력은 빌드 디렉터리의 `test-output/TVControllerGoldenMaster.received.txt`에 쓴다.
- 두 파일이 다르면 테스트를 실패시켜 의도하지 않은 출력 변경을 바로 확인한다.
- 동작 변경이 의도된 경우에만 `UPDATE_GOLDEN_MASTER=1`로 기준 파일을 재생성한다.

## 실행 방법

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Golden Master 테스트만 실행하려면 다음 명령을 사용한다.

```powershell
ctest --test-dir build -R TVControllerGoldenMaster --output-on-failure
```

기준 파일을 갱신하려면 테스트 실패 diff를 검토한 뒤 다음 명령을 실행한다.

```powershell
cmake --build build --target update_golden_master
```

## CI 통합

GitHub Actions는 `.github/workflows/ci.yml`에서 `cmake --build` 후 `ctest --output-on-failure`를 실행한다. CI에서는 `UPDATE_GOLDEN_MASTER`를 설정하지 않으므로 기준 파일이 자동 갱신되지 않고, 출력 변경은 실패로 보고된다.
