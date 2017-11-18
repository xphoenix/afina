# build service
set(SOURCE_FILES
    EngineTest.cpp
)

add_executable(runCoroutineTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runCoroutineTests Coroutine gtest gtest_main)

add_backward(runCoroutineTests)
add_test(runCoroutineTests runCoroutineTests)
