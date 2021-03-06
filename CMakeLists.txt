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
	enable_testing()
	add_subdirectory(tests)
endif()

set(Kunlaboro_HEADERS
	include/Kunlaboro/Kunlaboro.hpp

	include/Kunlaboro/Component.hpp
	include/Kunlaboro/Component.inl
	include/Kunlaboro/Config.hpp
	include/Kunlaboro/Entity.hpp
	include/Kunlaboro/Entity.inl
	include/Kunlaboro/EntitySystem.hpp
	include/Kunlaboro/EntitySystem.inl
	include/Kunlaboro/EventSystem.hpp
	include/Kunlaboro/EventSystem.inl
	include/Kunlaboro/Message.hpp
	include/Kunlaboro/Message.inl
	include/Kunlaboro/MessageSystem.hpp
	include/Kunlaboro/MessageSystem.inl
	include/Kunlaboro/ID.hpp
	include/Kunlaboro/Views.hpp
	include/Kunlaboro/Views.inl

	include/Kunlaboro/detail/ComponentPool.hpp
	include/Kunlaboro/detail/Delegate.hpp
	include/Kunlaboro/detail/DynamicBitfield.hpp
	include/Kunlaboro/detail/JobQueue.hpp
)

set(Kunlaboro_SOURCES
	source/Kunlaboro/Component.cpp
	source/Kunlaboro/Entity.cpp
	source/Kunlaboro/EntitySystem.cpp
	source/Kunlaboro/EventSystem.cpp
	source/Kunlaboro/Message.cpp
	source/Kunlaboro/MessageSystem.cpp
	source/Kunlaboro/Views.cpp

	source/Kunlaboro/detail/ComponentPool.cpp
	source/Kunlaboro/detail/DynamicBitfield.cpp
	source/Kunlaboro/detail/JobQueue.cpp
)

source_group("Header Files\\detail" FILES
	include/Kunlaboro/detail/ComponentPool.hpp
	include/Kunlaboro/detail/Delegate.hpp
	include/Kunlaboro/detail/DynamicBitfield.hpp
	include/Kunlaboro/detail/JobQueue.hpp
)
source_group("Source Files\\detail" FILES
	source/Kunlaboro/detail/ComponentPool.cpp
	source/Kunlaboro/detail/DynamicBitfield.cpp
	source/Kunlaboro/detail/JobQueue.cpp
)

add_library(Kunlaboro STATIC ${Kunlaboro_HEADERS} ${Kunlaboro_SOURCES})

if (WIN32 OR CYGWIN OR APPLE OR WITH_PIC OR DISABLE_SHARED OR NOT CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(_SKIP_PIC 1)
endif()

if (NOT _SKIP_PIC)
	set_target_properties(Kunlaboro PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")

	if (Kunlaboro_BUILD_TESTS)
		set_target_properties(testsuite PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
	endif()
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
