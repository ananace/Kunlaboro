project(Kunlaboro)

option(Kunlaboro_BOOST "Should Kunlaboro be built with boost (boost::any as message payloads)" OFF)

configure_file(Defines.hpp.in Defines.hpp)

set(Kunlaboro_SOURCES
	Component.cpp
	Component.hpp
	${CMAKE_CURRENT_BINARY_DIR}/Defines.hpp
	EntitySystem.cpp
	EntitySystem.hpp
	Kunlaboro.hpp
	Template.cpp
	Template.hpp
)

add_library(Kunlaboro STATIC ${Kunlaboro_SOURCES})

if (WIN32 OR CYGWIN OR APPLE OR WITH_PIC OR DISABLE_SHARED OR NOT CMAKE_SHARED_LIBRARY_C_FLAGS)
	set(_SKIP_PIC 1)
endif()

if (NOT _SKIP_PIC)
  set_target_properties(Kunlaboro PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}	
	${CMAKE_CURRENT_BINARY_DIR}
)
#link_directories(${CMAKE_CURRENT_BINARY_DIR})