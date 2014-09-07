cmake_minimum_required(VERSION 2.8)
project(Kunlaboro)

option(Kunlaboro_BOOST "Should Kunlaboro be built with boost (boost::any as message payloads)" OFF)
option(Kunlaboro_BUILD_TESTS "Build and run Kunlaboro tests?" OFF)

if (Kunlaboro_BOOST)
	FIND_PACKAGE(Boost REQUIRED)

	include_directories(
		${Boost_INCLUDE_DIR}
	)
endif()

if (Kunlaboro_BUILD_TESTS)
    if (NOT Kunlaboro_BOOST)
        message(FATAL_ERROR "The tests require Kunlaboro to be built with Boost.")
    endif()
	add_subdirectory(tests)
endif()

configure_file(
	include/Kunlaboro/Defines.hpp.in
	include/Kunlaboro/Defines.hpp
)

set(Kunlaboro_HEADERS
	include/Kunlaboro/Component.hpp
	include/Kunlaboro/EntitySystem.hpp
	include/Kunlaboro/Kunlaboro.hpp
)

set(Kunlaboro_SOURCES
	source/Kunlaboro/Component.cpp
	source/Kunlaboro/EntitySystem.cpp
)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Werror")
endif()

add_library(Kunlaboro STATIC ${Kunlaboro_HEADERS} ${Kunlaboro_SOURCES})

if (WIN32 OR CYGWIN OR APPLE OR WITH_PIC OR DISABLE_SHARED OR NOT CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(_SKIP_PIC 1)
endif()

if (NOT _SKIP_PIC)
  set_target_properties(Kunlaboro PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
)
