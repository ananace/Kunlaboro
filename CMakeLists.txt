cmake_minimum_required(VERSION 3.0)
project(Kunlaboro VERSION 0.2)

option(Kunlaboro_BUILD_TESTS "Build and run Kunlaboro tests?" OFF)

if (MSVC)
	if (${MSVC_VERSION} VERSION_LESS 1900)
		message(FATAL_ERROR "You need at least Visual Studio 2015 to compile Kunlaboro.")
	endif ()
elseif (UNIX)
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if (${GCC_VERSION} VERSION_GREATER 4.9 OR ${GCC_VERSION} VERSION_EQUAL 4.9)
			set(C11 "-std=c++14")
		else()
			message(FATAL_ERROR "You need at least GCC 4.9 to compile this project.")
		endif ()
	elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		message(WARNING "Assuming you have a recent enough Clang.")
		set(C11 "-std=c++14")
	endif ()

	set(CMAKE_CXX_FLAGS "${C11}")
endif ()

if (Kunlaboro_BUILD_TESTS)
	add_subdirectory(tests)
endif()

set(Kunlaboro_HEADERS
	include/Kunlaboro/Component.hpp
	include/Kunlaboro/Component.inl
    include/Kunlaboro/Config.hpp
	include/Kunlaboro/Entity.hpp
	include/Kunlaboro/Entity.inl
	include/Kunlaboro/EntitySystem.hpp
	include/Kunlaboro/EntitySystem.inl
	include/Kunlaboro/ID.hpp

	include/Kunlaboro/detail/ComponentPool.hpp
)

set(Kunlaboro_SOURCES
	source/Kunlaboro/Component.cpp
	source/Kunlaboro/Entity.cpp
	source/Kunlaboro/EntitySystem.cpp

	source/Kunlaboro/detail/ComponentPool.cpp
)

source_group("Header Files\\detail" FILES include/Kunlaboro/detail/ComponentPool.hpp)
source_group("Source Files\\detail" FILES source/Kunlaboro/detail/ComponentPool.cpp)

add_library(Kunlaboro STATIC ${Kunlaboro_HEADERS} ${Kunlaboro_SOURCES})

if (WIN32 OR CYGWIN OR APPLE OR WITH_PIC OR DISABLE_SHARED OR NOT CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(_SKIP_PIC 1)
endif()

if (NOT _SKIP_PIC)
  set_target_properties(Kunlaboro PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
