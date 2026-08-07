include_directories(SYSTEM "include")

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wno-deprecated-volatile
        -Wno-unqualified-std-cast-call
    )
endif ()

add_subdirectory(msvc_stl_p0323r12_test)
if (MSVC_TOOLSET_VERSION GREATER_EQUAL 143)
    # can only built with v143 and later
    add_subdirectory(msvc_stl_p2505r5_test)
endif ()
