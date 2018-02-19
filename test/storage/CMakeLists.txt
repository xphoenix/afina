# build service
set(SOURCE_FILES
    StorageTest.cpp
)

add_executable(runStorageTests ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(runStorageTests Storage gtest gtest_main)

add_backward(runStorageTests)
add_test(runStorageTests runStorageTests)
