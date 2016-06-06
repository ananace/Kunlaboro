SET(SRC
    main.cpp

    component.cpp
    comprehension.cpp
    entity.cpp
    messages.cpp
    speed.cpp
    system.cpp
    threading.cpp
)

find_package (Threads)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/tests")

add_executable(testsuite ${SRC})

target_link_libraries(testsuite Kunlaboro ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME testsuite
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND testsuite
)
