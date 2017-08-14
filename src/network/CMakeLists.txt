# build service
set(SOURCE_FILES
    Server.cpp
    Worker.cpp
)

add_library(Network ${SOURCE_FILES})
target_link_libraries(Network uv ${CMAKE_THREAD_LIBS_INIT})
