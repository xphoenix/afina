# build service
set(SOURCE_FILES
    MemcachedParserTest.cpp
)

add_executable(runProtocolTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runProtocolTests Protocol gtest gtest_main)

add_backward(runProtocolTests)
add_test(runProtocolTests runProtocolTests)
