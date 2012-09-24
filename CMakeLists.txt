project(Kunlaboro)

set(Kunlaboro_SOURCES
	Component.cpp
	Component.hpp
	Defines.hpp
	EntitySystem.cpp
	EntitySystem.hpp
	Kunlaboro.hpp
)

add_library(Kunlaboro STATIC ${Kunlaboro_SOURCES})

link_directories(${CMAKE_CURRENT_BINARY_DIR})