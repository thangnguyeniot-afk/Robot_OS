# save_test_log.cmake
# Helper script invoked by the save_test_log target.
# Runs ctest with full output and writes the result to logs/host_<date>.log.
#
# Variables passed in by add_custom_target:
#   LOG_DIR           — absolute path to tests/host/logs/
#   CTEST_BINARY_DIR  — build directory (where CTestTestfile.cmake lives)

cmake_minimum_required(VERSION 3.20)

string(TIMESTAMP TODAY "%Y-%m-%d")
set(LOG_FILE "${LOG_DIR}/host_${TODAY}.log")

execute_process(
    COMMAND ${CMAKE_CTEST_COMMAND}
        --test-dir "${CTEST_BINARY_DIR}"
        --output-on-failure
        --verbose
    OUTPUT_VARIABLE CTEST_OUTPUT
    ERROR_VARIABLE  CTEST_OUTPUT
    RESULT_VARIABLE CTEST_RESULT
)

file(WRITE "${LOG_FILE}" "${CTEST_OUTPUT}")
message(STATUS "Test log written to: ${LOG_FILE}")

if(NOT CTEST_RESULT EQUAL 0)
    message(FATAL_ERROR "One or more tests FAILED. Log saved to: ${LOG_FILE}")
endif()
