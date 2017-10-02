# build service
set(SOURCE_FILES
    Server.cpp
    Worker.cpp

    memcached/Parser.cpp
)

add_library(Network ${SOURCE_FILES})
target_link_libraries(Network uv Execute ${CMAKE_THREAD_LIBS_INIT})
