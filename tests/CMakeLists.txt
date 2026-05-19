include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)

add_executable(test_convert test_convert.cpp)
target_link_libraries(test_convert PRIVATE ezec gtest_main)
add_test(NAME test_convert COMMAND test_convert)

add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration PRIVATE ezec gtest_main)
target_compile_definitions(test_integration PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_integration COMMAND test_integration)
