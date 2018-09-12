# build service
set(SOURCE_FILES
    Parser.cpp
)

add_library(Protocol ${SOURCE_FILES})
target_link_libraries(Protocol Execute ${CMAKE_THREAD_LIBS_INIT})
