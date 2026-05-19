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
    src/FavoriteChannels.cpp
    src/TVController.cpp
)
target_link_libraries(TVControllerTest GTest::gtest_main GTest::gmock)

add_executable(FavoriteChannelTest 
    test/FavoriteChannelTest.cpp 
    src/remoteKey.cpp
    src/FavoriteChannels.cpp
    src/TVController.cpp
)
target_link_libraries(FavoriteChannelTest GTest::gtest_main GTest::gmock)

add_executable(TVControllerGoldenMasterTest
    test/TVControllerGoldenMasterTest.cpp
    src/remoteKey.cpp
    src/FavoriteChannels.cpp
    src/TVController.cpp
)
target_link_libraries(TVControllerGoldenMasterTest GTest::gtest_main GTest::gmock)
target_compile_definitions(TVControllerGoldenMasterTest PRIVATE
    GOLDEN_MASTER_APPROVED_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test/golden/TVControllerGoldenMaster.approved.txt"
    GOLDEN_MASTER_RECEIVED_FILE="${CMAKE_CURRENT_BINARY_DIR}/test-output/TVControllerGoldenMaster.received.txt"
)

# ============================================================
# ⭐ [Approval / Golden Master Test] 실행 경로 및 기준 파일 설정
# ============================================================

include(GoogleTest)

gtest_discover_tests(TunerTest)
gtest_discover_tests(TVControllerTest)
gtest_discover_tests(FavoriteChannelTest)
gtest_discover_tests(TVControllerGoldenMasterTest
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(update_golden_master
    COMMAND ${CMAKE_COMMAND} -E env UPDATE_GOLDEN_MASTER=1 $<TARGET_FILE:TVControllerGoldenMasterTest>
    DEPENDS TVControllerGoldenMasterTest
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Regenerating TVController Golden Master approved output"
)


add_custom_target(clear_tuner_received ALL
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_SOURCE_DIR}/test/TunerTest.received.txt"
)

# TunerTest가 빌드되기 전에 무조건 위 청소 명령이 먼저 실행되도록 순서 강제 의존성 지정
add_dependencies(TunerTest clear_tuner_received)