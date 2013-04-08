function(Kunlaboro_TEST Executable)
	add_executable(${Executable} ${ARGN})
	GTEST_ADD_TESTS(${Executable} "" ${ARGN})
	target_link_libraries(${Executable} Kunlaboro ${GTEST_BOTH_LIBRARIES})
	if (NOT WIN32)
		target_link_libraries(${Executable} pthreads)
	endif ()
endfunction()

enable_testing()
find_package(GTest REQUIRED)
include_directories(
	${GTEST_INCLUDE_DIRS}
	../..
	${CMAKE_CURRENT_BINARY_DIR}/..
)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

Kunlaboro_TEST(Test test.cpp)