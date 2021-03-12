# build service
set(SOURCE_FILES
        SimpleLRU.cpp
        StripedLRU.cpp
)

add_library(Storage ${SOURCE_FILES})
target_link_libraries(Storage ${CMAKE_THREAD_LIBS_INIT})
