# build service
set(SOURCE_FILES
    Command.cpp
)

add_library(Execute ${SOURCE_FILES})
target_link_libraries(Execute Storage ${CMAKE_THREAD_LIBS_INIT})
