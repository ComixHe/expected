include_guard(GLOBAL)

include(CheckCXXSourceRuns)

option(ZEUS_EXPECTED_TEST_ASAN_AUTO
    "Automatically enable AddressSanitizer for tests when supported"
    ON
)
option(ZEUS_EXPECTED_TEST_ASAN_REQUIRED
    "Require AddressSanitizer support for tests"
    OFF
)
option(ZEUS_EXPECTED_TEST_UBSAN_AUTO
    "Automatically enable UndefinedBehaviorSanitizer for tests when supported"
    ON
)
option(ZEUS_EXPECTED_TEST_UBSAN_REQUIRED
    "Require UndefinedBehaviorSanitizer support for tests"
    OFF
)
# REQUIRED takes precedence over AUTO for each sanitizer. With both options
# disabled, that sanitizer is not probed or enabled.

function(_zeus_expected_check_asan_support RESULT_VARIABLE)
    unset(_ZEUS_EXPECTED_TEST_ASAN_SUPPORTED CACHE)
    if (CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
        set(${RESULT_VARIABLE} FALSE PARENT_SCOPE)
        return()
    endif ()

    string(JOIN " " _required_flags
        ${_zeus_expected_test_asan_compile_options}
    )
    string(APPEND CMAKE_REQUIRED_FLAGS " ${_required_flags}")
    list(APPEND CMAKE_REQUIRED_LINK_OPTIONS
        ${_zeus_expected_test_asan_link_options}
    )

    check_cxx_source_runs([=[
        extern "C" const char* __asan_default_options()
        {
            return "detect_leaks=0";
        }

        int main()
        {
            return 0;
        }
    ]=]
        _ZEUS_EXPECTED_TEST_ASAN_SUPPORTED
    )
    set(${RESULT_VARIABLE}
        "${_ZEUS_EXPECTED_TEST_ASAN_SUPPORTED}"
        PARENT_SCOPE
    )
endfunction()

function(_zeus_expected_enable_asan RESULT_VARIABLE)
    set(${RESULT_VARIABLE} "" PARENT_SCOPE)

    if (NOT (ZEUS_EXPECTED_TEST_ASAN_AUTO
        OR ZEUS_EXPECTED_TEST_ASAN_REQUIRED))
        message(STATUS "AddressSanitizer disabled for tests")
        return()
    endif ()

    if (MSVC)
        set(_zeus_expected_test_asan_compile_options /fsanitize=address)
        set(_zeus_expected_test_asan_link_options)
    else ()
        set(_zeus_expected_test_asan_compile_options
            -fsanitize=address
            -fno-omit-frame-pointer
        )
        set(_zeus_expected_test_asan_link_options -fsanitize=address)
    endif ()

    _zeus_expected_check_asan_support(
        ZEUS_EXPECTED_TEST_ASAN_SUPPORTED
    )

    if (ZEUS_EXPECTED_TEST_ASAN_SUPPORTED)
        add_compile_options(${_zeus_expected_test_asan_compile_options})
        if (_zeus_expected_test_asan_link_options)
            add_link_options(${_zeus_expected_test_asan_link_options})
        endif ()

        if (WIN32)
            set(_environment "ASAN_OPTIONS=halt_on_error=1")
        else ()
            add_library(zeus_expected_asan_defaults OBJECT
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/asan_default_options.cpp"
            )
            link_libraries(zeus_expected_asan_defaults)

            set(_environment
                "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1"
            )
        endif ()
        set(${RESULT_VARIABLE} "${_environment}" PARENT_SCOPE)
        message(STATUS "AddressSanitizer enabled for tests")
    elseif (ZEUS_EXPECTED_TEST_ASAN_REQUIRED)
        message(FATAL_ERROR
            "ZEUS_EXPECTED_TEST_ASAN_REQUIRED=ON, but the current compiler, "
            "linker, or runtime environment cannot run "
            "AddressSanitizer-instrumented test executables"
        )
    else ()
        message(STATUS
            "AddressSanitizer unavailable; tests will run without it"
        )
    endif ()
endfunction()

function(_zeus_expected_check_ubsan_support RESULT_VARIABLE)
    unset(_ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED CACHE)
    if (CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
        set(${RESULT_VARIABLE} FALSE PARENT_SCOPE)
        return()
    endif ()

    string(JOIN " " _required_flags
        ${_zeus_expected_test_ubsan_compile_options}
    )
    string(APPEND CMAKE_REQUIRED_FLAGS " ${_required_flags}")
    list(APPEND CMAKE_REQUIRED_LINK_OPTIONS
        ${_zeus_expected_test_ubsan_link_options}
    )

    # Keep a safe division check in the executable so the linker must resolve
    # the UBSan runtime, then run it to verify that the runtime is loadable.
    check_cxx_source_runs([=[
        int main(int argc, char**)
        {
            volatile int divisor = argc;
            const int result = 1 / divisor;
            return result == 1 ? 0 : 1;
        }
    ]=]
        _ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED
    )
    set(${RESULT_VARIABLE}
        "${_ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED}"
        PARENT_SCOPE
    )
endfunction()

function(_zeus_expected_enable_ubsan RESULT_VARIABLE)
    set(${RESULT_VARIABLE} "" PARENT_SCOPE)

    if (NOT (ZEUS_EXPECTED_TEST_UBSAN_AUTO
        OR ZEUS_EXPECTED_TEST_UBSAN_REQUIRED))
        message(STATUS "UndefinedBehaviorSanitizer disabled for tests")
        return()
    endif ()

    if (CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
        set(_zeus_expected_test_ubsan_compile_options
            -fsanitize=undefined
            -fno-sanitize-recover=undefined
            -fno-omit-frame-pointer
        )
        if (CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
            list(APPEND _zeus_expected_test_ubsan_compile_options
                -fno-sanitize-merge
            )
        endif ()
        set(_zeus_expected_test_ubsan_link_options
            -fsanitize=undefined
        )

        _zeus_expected_check_ubsan_support(
            ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED
        )
    else ()
        set(ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED FALSE)
    endif ()

    if (ZEUS_EXPECTED_TEST_UBSAN_SUPPORTED)
        add_compile_options(${_zeus_expected_test_ubsan_compile_options})
        add_link_options(${_zeus_expected_test_ubsan_link_options})

        set(${RESULT_VARIABLE}
            "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1"
            PARENT_SCOPE
        )
        message(STATUS "UndefinedBehaviorSanitizer enabled for tests")
    elseif (ZEUS_EXPECTED_TEST_UBSAN_REQUIRED)
        message(FATAL_ERROR
            "ZEUS_EXPECTED_TEST_UBSAN_REQUIRED=ON, but the current compiler, "
            "linker, or runtime environment cannot run "
            "UndefinedBehaviorSanitizer-instrumented test executables"
        )
    else ()
        message(STATUS
            "UndefinedBehaviorSanitizer unavailable; tests will run without it"
        )
    endif ()
endfunction()

function(zeus_expected_enable_test_sanitizers)
    _zeus_expected_enable_asan(_asan_environment)
    _zeus_expected_enable_ubsan(_ubsan_environment)

    set(_environment)
    if (_asan_environment)
        list(APPEND _environment "${_asan_environment}")
    endif ()
    if (_ubsan_environment)
        list(APPEND _environment "${_ubsan_environment}")
    endif ()
    set(_ZEUS_EXPECTED_TEST_SANITIZER_ENVIRONMENT
        "${_environment}"
        PARENT_SCOPE
    )
endfunction()

function(zeus_expected_configure_test_sanitizers TEST_NAME)
    if (_ZEUS_EXPECTED_TEST_SANITIZER_ENVIRONMENT)
        set_tests_properties("${TEST_NAME}"
            PROPERTIES
                ENVIRONMENT "${_ZEUS_EXPECTED_TEST_SANITIZER_ENVIRONMENT}"
        )
    endif ()
endfunction()

function(zeus_expected_configure_catch_test_sanitizers TARGET_NAME)
    if (NOT _ZEUS_EXPECTED_TEST_SANITIZER_ENVIRONMENT)
        return()
    endif ()

    # Catch2 exposes the discovered test list only while CTest processes
    # TEST_INCLUDE_FILES. Set the environment there so its semicolon-separated
    # entries are not flattened as Catch2 property arguments.
    set(_sanitizer_script
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_sanitizer.cmake"
    )
    string(CONCAT _sanitizer_script_content
        "if(DEFINED ${TARGET_NAME}_TESTS)\n"
        "    set_tests_properties(\${${TARGET_NAME}_TESTS}\n"
        "        PROPERTIES ENVIRONMENT "
        "[==[${_ZEUS_EXPECTED_TEST_SANITIZER_ENVIRONMENT}]==]\n"
        "    )\n"
        "endif()\n"
    )
    file(GENERATE
        OUTPUT "${_sanitizer_script}"
        CONTENT "${_sanitizer_script_content}"
    )
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
        "${_sanitizer_script}"
    )
endfunction()
