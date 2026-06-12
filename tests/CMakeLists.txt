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
target_link_libraries(test_convert PRIVATE pace gtest_main)
add_test(NAME test_convert COMMAND test_convert)

add_executable(test_ao test_ao.cpp)
target_link_libraries(test_ao PRIVATE pace gtest_main)
target_compile_definitions(test_ao PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_ao COMMAND test_ao)

add_executable(test_longout test_longout.cpp)
target_link_libraries(test_longout PRIVATE pace gtest_main)
target_compile_definitions(test_longout PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_longout COMMAND test_longout)

add_executable(test_stringout test_stringout.cpp)
target_link_libraries(test_stringout PRIVATE pace gtest_main)
target_compile_definitions(test_stringout PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_stringout COMMAND test_stringout)

add_executable(test_mbbo test_mbbo.cpp)
target_link_libraries(test_mbbo PRIVATE pace gtest_main)
target_compile_definitions(test_mbbo PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_mbbo COMMAND test_mbbo)

add_executable(test_waveform test_waveform.cpp)
target_link_libraries(test_waveform PRIVATE pace gtest_main)
target_compile_definitions(test_waveform PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_waveform COMMAND test_waveform)

add_executable(test_context test_context.cpp)
target_link_libraries(test_context PRIVATE pace gtest_main)
target_compile_definitions(test_context PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_context COMMAND test_context)

add_executable(test_pva test_pva.cpp)
target_link_libraries(test_pva PRIVATE pace gtest_main)
target_compile_definitions(test_pva PRIVATE
    EPICS_BASE="${EPICS_BASE}"
    EPICS_HOST_ARCH="${EPICS_HOST_ARCH}"
    TEST_DB_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME test_pva COMMAND test_pva)
