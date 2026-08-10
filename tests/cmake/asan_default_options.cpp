// Catch2 runs test executables during the build to discover test cases. Leak
// detection is enabled again by the ASAN_OPTIONS attached to the CTest tests.
extern "C" const char* __asan_default_options()
{
    return "detect_leaks=0";
}
