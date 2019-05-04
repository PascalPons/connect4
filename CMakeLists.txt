CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(CONNECT4)
SET(CONNECT4_VERSION_MAJOR 0)
SET(CONNECT4_VERSION_MINOR 9)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -W -Wall -O3 -DNDEBUG")

ADD_LIBRARY(c4solver_dependencies STATIC
	Solver.cpp
	Solver.hpp
	Position.hpp
	TranspositionTable.hpp
        OpeningBook.hpp
	MoveSorter.hpp)

ADD_EXECUTABLE(c4solver
	main.cpp)

ADD_EXECUTABLE(c4generator
        generator.cpp)

TARGET_LINK_LIBRARIES(c4solver c4solver_dependencies)
TARGET_LINK_LIBRARIES(c4generator c4solver_dependencies)

INSTALL(TARGETS c4solver_dependencies DESTINATION lib)
INSTALL(TARGETS c4generator DESTINATION bin)
INSTALL(TARGETS c4solver DESTINATION bin)
