cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(CGAL REQUIRED) # https://github.com/CGAL/cgal

add_executable(CGALMeshSimplificationTest
        main.cpp
)

target_link_libraries(CGALMeshSimplificationTest CGAL::CGAL)