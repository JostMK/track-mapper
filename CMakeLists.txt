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
        src/web/TestWebApp.cpp
)

# 'ws2_32' 'wsock32' needed when using MINGW on windows: https://github.com/CrowCpp/Crow/issues/759
find_package(Crow CONFIG REQUIRED)
target_link_libraries(track_mapper PRIVATE Crow::Crow asio::asio ws2_32 wsock32)