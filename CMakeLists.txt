cmake_minimum_required(VERSION 3.28)
project(track_mapper)

set(CMAKE_CXX_STANDARD 23)

add_executable(track_mapper main.cpp
        src/graphs/IGraph.h
        src/graphs/DijkstraPathfinding.h
        src/graphs/DijkstraPathfinding.cpp
        src/graphs/BasicGraph.h
        src/graphs/BasicGraph.cpp
        src/graphs/FMIGraphReader.cpp
)