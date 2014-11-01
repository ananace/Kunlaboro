cmake_minimum_required(VERSION 2.8)
project(Kunlaboro)

option(Kunlaboro_BUILD_TESTS "Build and run Kunlaboro tests?" OFF)

if (MSVC)
	if (${MSVC_VERSION} VERSION_LESS 1700)
		message(FATAL_ERROR "You need to compile this project with at least Visual Studio 2012 it provides the C++11 subset Kunlaboro needs.")
    elseif (${MSVC_VERSION} VERSION_EQUAL 1800 AND NOT ${CMAKE_VS_PLATFORM_TOOLSET} EQUAL "CTP_Nov2013")
        message(STATUS "Note that with the 2013 November CTP you can improve performance of Kunlaboro, since that CTP supports constexpr.")
	endif ()
    add_definitions("-D_MSC_PLATFORM_TOOLSET_$(PlatformToolset)")
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

if (Kunlaboro_BUILD_TESTS)
	add_subdirectory(tests)
endif()

set(Kunlaboro_HEADERS
	include/Kunlaboro/Component.hpp
    include/Kunlaboro/Defines.hpp
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
