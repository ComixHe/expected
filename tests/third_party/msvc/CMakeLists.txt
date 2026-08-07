include_directories(SYSTEM "include")

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wno-deprecated-volatile
        -Wno-unqualified-std-cast-call
    )
endif ()

add_subdirectory(msvc_stl_p0323r12_test)
if (NOT MSVC OR MSVC_TOOLSET_VERSION GREATER_EQUAL 143)
    # The MSVC build requires v143 or later. GCC and Clang are already
    # version-gated by the parent directory.
    add_subdirectory(msvc_stl_p2505r5_test)
endif ()
