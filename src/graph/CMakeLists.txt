cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

add_library(TrackMapperGraphLib STATIC
        IGraph.h
        DijkstraPathfinding.h
        DijkstraPathfinding.cpp
        BasicGraph.h
        BasicGraph.cpp
        FMIGraphReader.h
        FMIGraphReader.cpp
        IGrid.h
        SimpleWorldGrid.h
        SimpleWorldGrid.cpp
)

add_executable(TrackMapperGraphConsoleApp
        main.cpp
)
target_link_libraries(TrackMapperGraphConsoleApp PRIVATE TrackMapperGraphLib)