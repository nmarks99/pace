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

add_executable(test_ao test_ao.cpp)
target_link_libraries(test_ao PRIVATE ezec gtest_main)
target_compile_definitions(test_ao PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_ao COMMAND test_ao)

add_executable(test_longout test_longout.cpp)
target_link_libraries(test_longout PRIVATE ezec gtest_main)
target_compile_definitions(test_longout PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_longout COMMAND test_longout)
