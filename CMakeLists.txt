cmake_minimum_required(VERSION 3.14)

project(ezec LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- EPICS Base ---------------------------------------------------------------
if(NOT DEFINED EPICS_BASE OR EPICS_BASE STREQUAL "")
    set(EPICS_BASE $ENV{EPICS_BASE})
endif()
if(NOT EPICS_BASE OR EPICS_BASE STREQUAL "")
    message(FATAL_ERROR "EPICS_BASE not set. Pass -DEPICS_BASE=... or set the environment variable.")
endif()
set(EPICS_BASE "${EPICS_BASE}" CACHE STRING "Path to EPICS base")

set(EPICS_HOST_ARCH "$ENV{EPICS_HOST_ARCH}" CACHE STRING "Host architecture")
if(NOT EPICS_HOST_ARCH)
    message(FATAL_ERROR "'EPICS_HOST_ARCH' not defined")
endif()

set(EPICS_LIB_DIR "${EPICS_BASE}/lib/${EPICS_HOST_ARCH}")
foreach(LIB ca Com)
    find_library(EPICS_${LIB} ${LIB}
        PATHS ${EPICS_LIB_DIR}
        NO_DEFAULT_PATH
        REQUIRED
    )
endforeach()

set(POTENTIAL_EPICS_INCLUDES
    ${EPICS_BASE}/include
    ${EPICS_BASE}/include/compiler/gcc
    ${EPICS_BASE}/include/compiler/clang
    ${EPICS_BASE}/include/os/Linux
    ${EPICS_BASE}/include/os/Darwin
)
set(EPICS_BASE_INCLUDES "")
foreach(DIR ${POTENTIAL_EPICS_INCLUDES})
    if(IS_DIRECTORY "${DIR}")
        list(APPEND EPICS_BASE_INCLUDES "${DIR}")
    endif()
endforeach()
# ------------------------------------------------------------------------------

# --- ezec header-only library -------------------------------------------------
add_library(ezec INTERFACE)
target_include_directories(ezec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ezec SYSTEM INTERFACE ${EPICS_BASE_INCLUDES})
target_link_libraries(ezec INTERFACE ${EPICS_ca} ${EPICS_Com})
# ------------------------------------------------------------------------------

# --- Tests --------------------------------------------------------------------
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
# ------------------------------------------------------------------------------
