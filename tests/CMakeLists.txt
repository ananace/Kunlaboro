enable_testing()

find_package(GTest REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread)

include_directories(
	${GTEST_INCLUDE_DIRS}
	../include
	${CMAKE_CURRENT_BINARY_DIR}/../include
)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_executable(Test "test.cpp")
GTEST_ADD_TESTS(Test "" "test.cpp")
target_link_libraries(Test
	Kunlaboro
	${GTEST_BOTH_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${Boost_LIBRARIES}
)