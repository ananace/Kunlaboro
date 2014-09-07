cmake_minimum_required(VERSION 2.8)
project(Kunlaboro)

option(Kunlaboro_BOOST "Should Kunlaboro be built with boost (boost::any as message payloads)" OFF)
option(Kunlaboro_BUILD_TESTS "Build and run Kunlaboro tests?" OFF)

if (MSVC)
	if (${MSVC_VERSION} VERSION_LESS 11.0)
		message(FATAL_ERROR "You need to compile this project with at least Visual Studio 2012, 2010 does not provide the entire C++11 subset Kunlaboro needs")
	endif ()
elseif (UNIX)
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if (${GCC_VERSION} VERSION_GREATER 4.7 OR ${GCC_VERSION} VERSION_EQUAL 4.7)
			set(C11 "-std=c++11")
		elseif (${GCC_VERSION} VERSION_GREATER 4.3 OR ${GCC_VERSION} VERSION_EQUAL 4.3)
			set(C11 "-std=c++0x")
		else ()
			message(FATAL_ERROR "You need at least GCC 4.3 to compile this project.")
		endif ()
	elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		message(WARNING "Assuming you have a recent enough Clang.")
		set(C11 "-std=c++11")
	endif ()

	set(CMAKE_CXX_FLAGS "${C11}")
endif ()

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
