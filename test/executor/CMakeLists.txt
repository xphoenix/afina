# build service
set(SOURCE_FILES
	ExecutorTest.cpp
)

add_executable(runExecutorTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runExecutorTests Executor gtest gmock gmock_main)

add_backward(runExecutorTests)
add_test(runExecutorTests runExecutorTests)
