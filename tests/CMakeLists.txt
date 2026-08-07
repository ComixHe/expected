include("${CMAKE_CURRENT_LIST_DIR}/cmake/ZeusExpectedTestSanitizers.cmake")

zeus_expected_enable_test_sanitizers()

add_subdirectory(test_expected)
add_subdirectory(test_no_exceptions)
add_subdirectory(third_party)
