# build service
<<<<<<< HEAD
set(SOURCE_FILES )
=======
set(SOURCE_FILES
    MemcachedParserTest.cpp
)
>>>>>>> 29ea0e73f97d6f181583d2b8d44eec2888afe943

add_executable(runNetworkTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runNetworkTests Network gtest gtest_main)

add_backward(runNetworkTests)
add_test(runNetworkTests runNetworkTests)
