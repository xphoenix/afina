# build service
set(SOURCE_FILES
    SimpleTest.cpp
)

add_executable(runAllocatorTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runAllocatorTests Allocator gtest gtest_main)

add_backward(runAllocatorTests)
add_test(runAllocatorTests runAllocatorTests)
