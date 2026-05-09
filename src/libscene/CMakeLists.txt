cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(glaze CONFIG REQUIRED)

add_library(LibScene STATIC
        Mesh.h
        Mesh.cpp
        Scene.h
        Scene.cpp
)

target_link_libraries(LibScene PRIVATE glaze::glaze)