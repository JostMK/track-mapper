cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

add_executable(TrackMapperConsoleApp
        main.cpp
)
target_link_libraries(TrackMapperConsoleApp PRIVATE TrackMapperMeshLib TrackMapperSceneLib)