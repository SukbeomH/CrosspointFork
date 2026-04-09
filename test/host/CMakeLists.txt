cmake_minimum_required(VERSION 3.16)
project(CrossPointHostTests CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# -------------------------------------------------------------------
# Test build flag — guards mock paths in production headers
# -------------------------------------------------------------------
add_definitions(-DUNIT_TEST)

# -------------------------------------------------------------------
# Paths relative to this file (test/host/)
# -------------------------------------------------------------------
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(MOCK_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/mocks")

# -------------------------------------------------------------------
# Include paths: mocks first so our stubs override ESP32 headers
# -------------------------------------------------------------------
include_directories(
  ${MOCK_DIR}                        # Arduino.h, FreeRTOS stubs, HalStorage mock
  ${PROJECT_ROOT}/lib/ScriptDetector
  ${PROJECT_ROOT}/lib/Utf8
  ${PROJECT_ROOT}/lib/Serialization
  ${PROJECT_ROOT}/src
)

# -------------------------------------------------------------------
# Mock / stub sources (compiled once, linked to every test)
# -------------------------------------------------------------------
add_library(mocks STATIC
  ${MOCK_DIR}/Arduino.cpp
  ${MOCK_DIR}/HalStorageMock.cpp
)

# -------------------------------------------------------------------
# Real library sources used by tests
# -------------------------------------------------------------------
add_library(utf8_lib STATIC
  ${PROJECT_ROOT}/lib/Utf8/Utf8.cpp
)

add_library(script_detector_lib STATIC
  ${PROJECT_ROOT}/lib/ScriptDetector/ScriptDetector.cpp
)
target_link_libraries(script_detector_lib PUBLIC utf8_lib)

# -------------------------------------------------------------------
# Enable CTest
# -------------------------------------------------------------------
enable_testing()

# -------------------------------------------------------------------
# Helper macro — define a host test target
# -------------------------------------------------------------------
macro(add_host_test TEST_NAME TEST_SRC)
  add_executable(${TEST_NAME} ${TEST_SRC})
  target_link_libraries(${TEST_NAME} PRIVATE mocks utf8_lib script_detector_lib)
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

# -------------------------------------------------------------------
# Test targets
# -------------------------------------------------------------------
add_host_test(test_ScriptDetector   "${CMAKE_CURRENT_SOURCE_DIR}/test_ScriptDetector.cpp")
add_host_test(test_BookmarkStore    "${CMAKE_CURRENT_SOURCE_DIR}/test_BookmarkStore.cpp")
add_host_test(test_BookSettings     "${CMAKE_CURRENT_SOURCE_DIR}/test_BookSettings.cpp")
add_host_test(test_ReadingStats     "${CMAKE_CURRENT_SOURCE_DIR}/test_ReadingStats.cpp")
add_host_test(test_AchievementStore "${CMAKE_CURRENT_SOURCE_DIR}/test_AchievementStore.cpp")
