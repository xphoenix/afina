# build service
set(SOURCE_FILES
    st_blocking/ServerImpl.cpp
    mt_blocking/ServerImpl.cpp

    st_nonblocking/ServerImpl.cpp
    st_nonblocking/Connection.cpp
    st_nonblocking/Utils.cpp

    st_coroutine/ServerImpl.cpp
    st_coroutine/Connection.cpp
    st_coroutine/Utils.cpp

    mt_nonblocking/ServerImpl.cpp
    mt_nonblocking/Connection.cpp
    mt_nonblocking/Worker.cpp
    mt_nonblocking/Utils.cpp
)

add_library(Network ${SOURCE_FILES})
target_link_libraries(Network pthread Logging Protocol Concurrency Execute Coroutine ${CMAKE_THREAD_LIBS_INIT})
