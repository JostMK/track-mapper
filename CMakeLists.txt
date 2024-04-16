cmake_minimum_required(VERSION 3.28)
project(track_mapper)

set(CMAKE_CXX_STANDARD 23)

add_executable(track_mapper main.cpp
        src/graphs/Graph.h
        src/graphs/FMIGraph.h
        src/graphs/FMIGraph.cpp
)