function(Kunlaboro_TEST Executable)
	add_executable(${Executable} ${ARGN})
	GTEST_ADD_TESTS(${Executable} "" ${ARGN})
	target_link_libraries(${Executable} Kunlaboro ${GTEST_BOTH_LIBRARIES})
endfunction()

enable_testing()
find_package(GTest REQUIRED)
include_directories(
	${GTEST_INCLUDE_DIRS}
	../..
	${CMAKE_CURRENT_BINARY_DIR}/..
)

Kunlaboro_TEST(Test test.cpp)