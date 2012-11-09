project(Kunlaboro)

SET(Kunlaboro_BOOST True CACHE BOOL "Should Kunlaboro be built with boost (boost::any as message payloads)")

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

include_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_CURRENT_BINARY_DIR})