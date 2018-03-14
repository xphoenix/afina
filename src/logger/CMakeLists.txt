# build service
set(SOURCE_FILES Logger.h)

add_library(Logger ${SOURCE_FILES})
target_link_libraries(Logger ${CMAKE_THREAD_LIBS_INIT})
