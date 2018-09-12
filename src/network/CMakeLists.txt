# build service
set(SOURCE_FILES
    st_blocking/ServerImpl.cpp
    mt_blocking/ServerImpl.cpp
    nonblocking/ServerImpl.cpp
    nonblocking/Connection.cpp
    nonblocking/Worker.cpp
    nonblocking/Utils.cpp
)

add_library(Network ${SOURCE_FILES})
target_link_libraries(Network pthread Logging Protocol Execute ${CMAKE_THREAD_LIBS_INIT})
