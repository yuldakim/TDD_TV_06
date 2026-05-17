cmake_minimum_required(VERSION 3.14)
project(bestreviewer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 빌드 타입이 Debug일 때만 커버리지 옵션을 활성화하도록 설정
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling Code Coverage for Debug build")
    
    # GCC/Clang 컴파일러에 커버리지 플래그 추가
    add_compile_options(--coverage -O0)
    add_link_options(--coverage)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY OFF
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(include)

enable_testing()

# ============================================================
# 🧪 각 테스트 타깃에 새로 분리한 src/remoteKey.cpp 를 추가합니다.
# ============================================================

add_executable(TunerTest 
    test/TunerTest.cpp 
    src/remoteKey.cpp
)
target_link_libraries(TunerTest GTest::gtest_main GTest::gmock)

add_executable(TVControllerTest 
    test/TVControllerTest.cpp 
    src/remoteKey.cpp
    src/TVController.cpp
)
target_link_libraries(TVControllerTest GTest::gtest_main GTest::gmock)

add_executable(FavoriteChannelTest 
    test/FavoriteChannelTest.cpp 
    src/remoteKey.cpp
    src/TVController.cpp
)
target_link_libraries(FavoriteChannelTest GTest::gtest_main GTest::gmock)


# ============================================================
# ⭐ [Approval Test] 실행 경로 및 자산 복사 설정
# ============================================================

# 테스트가 실행될 때 .approved.txt 파일이 들어있는 위치를 제대로 인식하도록 
# 실행 디렉터리(Working Directory)를 소스 디렉터리로 강제 지정합니다.
if(COMMAND gtest_discover_tests)
    # VS Code 테스트 탐색기나 CTest 환경에서도 경로가 꼬이지 않게 잡아줍니다.
    set_tests_properties(FavoriteChannelTest PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test"
    )
endif()

include(GoogleTest)

gtest_discover_tests(TunerTest)
gtest_discover_tests(TVControllerTest)
gtest_discover_tests(FavoriteChannelTest)


add_custom_target(clear_tuner_received ALL
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_SOURCE_DIR}/test/TunerTest.received.txt"
)

# TunerTest가 빌드되기 전에 무조건 위 청소 명령이 먼저 실행되도록 순서 강제 의존성 지정
add_dependencies(TunerTest clear_tuner_received)