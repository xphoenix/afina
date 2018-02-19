cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(afina LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror -fPIC")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_THREAD_PREFER_PTHREAD)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)

# If system has ccache - use it
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Use native optimizations, for example fast crc32
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_OPT_ARCH_NATIVE_SUPPORTED)
if (COMPILER_OPT_ARCH_NATIVE_SUPPORTED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

##############################################################################
# Dependencies
##############################################################################
## GTest
set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject")
set(gtest_disable_pthreads ON CACHE BOOL "Disable uses of pthreads in gtest.")
add_subdirectory(third-party/googletest-release-1.8.0)

## Libuv
add_subdirectory(third-party/libuv-1.11.0)

## Stack traces
add_subdirectory(third-party/backward-cpp)

## Command line arguments parsing
set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "Set to ON to build examples")
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to build examples")
add_subdirectory(third-party/cxxopts-1.4.3)

##############################################################################
# Setup build system
##############################################################################
# Generate version information
IF (NOT AFINA_VERSION)
    include(GetGitRevisionDescription)
    git_local_changes(AFINA_VERSION_DIRTY)
    git_describe(AFINA_VERSION --long --tags --match "v[0-9]*" --abbrev=4)
ENDIF()

# major.minor.patch part of version
MESSAGE(STATUS "Version: " ${AFINA_VERSION} )
if ("${AFINA_VERSION}" MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+")
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${AFINA_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+)\\..*" "\\1" VERSION_MINOR "${AFINA_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${AFINA_VERSION}")
else()
    message(FATAL_ERROR "Version has wrong format, check major/minor/patch parts of version")
endif()

# additional git part of version
if ("${AFINA_VERSION}" MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-[a-zA-Z0-9]+")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-([0-9]+).*" "\\1" VERSION_COMMITS "${AFINA_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-(.*)" "\\1" VERSION_SHA1 "${AFINA_VERSION}")
else()
    message(WARNING "Version has no full git info")
    set(VERSION_COMMITS "")
    set(VERSION_SHA1 "")
endif()

MESSAGE( STATUS "VERSION_MAJOR: " ${VERSION_MAJOR} )
MESSAGE( STATUS "VERSION_MINOR: " ${VERSION_MINOR} )
MESSAGE( STATUS "VERSION_PATCH: " ${VERSION_PATCH} )
MESSAGE( STATUS "VERSION_COMMITS: " ${VERSION_COMMITS} )
MESSAGE( STATUS "VERSION_SHA1: " ${VERSION_SHA1} )
MESSAGE( STATUS "VERSION_DIRTY: " ${AFINA_VERSION_DIRTY} )


##############################################################################
# Sources
##############################################################################
## Build services
add_subdirectory(src)

## Build tests
enable_testing()
add_subdirectory(test)
