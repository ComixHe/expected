set(_zeus_expected_enable_msvc_stl_tests OFF)
if (MSVC)
    set(_zeus_expected_enable_msvc_stl_tests ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
    set(_zeus_expected_enable_msvc_stl_tests ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 22)
    set(_zeus_expected_enable_msvc_stl_tests ON)
endif ()

if (_zeus_expected_enable_msvc_stl_tests)
    add_subdirectory(msvc)
endif ()

unset(_zeus_expected_enable_msvc_stl_tests)
