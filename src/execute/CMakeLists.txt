# build service
set(SOURCE_FILES
    Command.cpp
    Add.cpp
    AppendPrepend.cpp
    InsertCommand.cpp
    IncrDecr.cpp
    MultipleStringsCommand.cpp
    Delete.cpp
    KeyValueCommand.cpp	
    Get.cpp
    Set.cpp
    Replace.cpp
    Stats.cpp
)

add_library(Execute ${SOURCE_FILES})
target_link_libraries(Execute Storage ${CMAKE_THREAD_LIBS_INIT})
