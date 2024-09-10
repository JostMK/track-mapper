cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(src/graph)
add_subdirectory(src/mesh)
add_subdirectory(src/scene)
add_subdirectory(src/web)

add_subdirectory(experiments)