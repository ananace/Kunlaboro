SET(SRCROOT "${PROJECT_SOURCE_DIR}/tests")

SET(SRC
    "${SRCROOT}/main.cpp"

    "${SRCROOT}/component.cpp"
    "${SRCROOT}/entity.cpp"
    "${SRCROOT}/speed.cpp"
    "${SRCROOT}/system.cpp"
    "${SRCROOT}/threading.cpp"
)

find_package (Threads)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/tests")

add_executable(testsuite ${SRC})

target_link_libraries(testsuite Kunlaboro ${CMAKE_THREAD_LIBS_INIT})

add_custom_target(run_tests
	DEPENDS testsuite
	WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/tests"
	COMMAND ${CMAKE_COMMAND} -E echo_append "Running tests...\n"
	COMMAND testsuite -d yes
)