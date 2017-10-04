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
set(LIBUVDIR third-party/libuv-1.11.0)

include_directories(${LIBUVDIR}/include ${LIBUVDIR}/src)
set(SOURCES
  ${LIBUVDIR}/src/fs-poll.c
  ${LIBUVDIR}/src/inet.c
  ${LIBUVDIR}/src/threadpool.c
  ${LIBUVDIR}/src/uv-common.c
  ${LIBUVDIR}/src/version.c)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-D_GNU_SOURCE)
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/linux-syscalls.c
    ${LIBUVDIR}/src/unix/linux-core.c
    ${LIBUVDIR}/src/unix/linux-inotify.c)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -D_DARWIN_UNLIMITED_SELECT=1)
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/darwin.c
    ${LIBUVDIR}/src/unix/darwin-proctitle.c
    ${LIBUVDIR}/src/unix/fsevents.c
    ${LIBUVDIR}/src/unix/kqueue.c
    ${LIBUVDIR}/src/unix/pthread-barrier.c
    ${LIBUVDIR}/src/unix/proctitle.c)
endif()

include_directories(${LIBUVDIR}/src/unix)
set(SOURCES ${SOURCES}
  ${LIBUVDIR}/src/unix/async.c
  ${LIBUVDIR}/src/unix/core.c
  ${LIBUVDIR}/src/unix/dl.c
  ${LIBUVDIR}/src/unix/fs.c
  ${LIBUVDIR}/src/unix/getaddrinfo.c
  ${LIBUVDIR}/src/unix/getnameinfo.c
  ${LIBUVDIR}/src/unix/loop-watcher.c
  ${LIBUVDIR}/src/unix/loop.c
  ${LIBUVDIR}/src/unix/pipe.c
  ${LIBUVDIR}/src/unix/poll.c
  ${LIBUVDIR}/src/unix/process.c
  ${LIBUVDIR}/src/unix/signal.c
  ${LIBUVDIR}/src/unix/stream.c
  ${LIBUVDIR}/src/unix/tcp.c
  ${LIBUVDIR}/src/unix/thread.c
  ${LIBUVDIR}/src/unix/timer.c
  ${LIBUVDIR}/src/unix/tty.c
  ${LIBUVDIR}/src/unix/udp.c)

add_library(uv STATIC ${SOURCES})

target_link_libraries(uv pthread)

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
