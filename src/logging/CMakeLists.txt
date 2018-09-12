# build service
set(SOURCE_FILES
    ServiceImpl.cpp
)

add_library(Logging ${SOURCE_FILES})
target_link_libraries(Logging spdlog ${CMAKE_THREAD_LIBS_INIT})
