# build service
set(SOURCE_FILES
)

add_executable(runExecuteTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runExecuteTests Execute gtest gmock gmock_main)

add_backward(runExecuteTests)
add_test(runExecuteTests runExecuteTests)
