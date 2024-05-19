cmake_minimum_required(VERSION 3.28)
project(track_mapper)

set(CMAKE_CXX_STANDARD 23)

add_executable(track_mapper
        main.cpp
        src/graphs/IGraph.h
        src/graphs/DijkstraPathfinding.h
        src/graphs/DijkstraPathfinding.cpp
        src/graphs/BasicGraph.h
        src/graphs/BasicGraph.cpp
        src/graphs/FMIGraphReader.cpp
        src/web/TestWebApp.cpp
        src/graphs/IGrid.h
        src/graphs/SimpleWorldGrid.h
        src/graphs/SimpleWorldGrid.cpp
)

add_dependencies(track_mapper Webserver)

# 'ws2_32' 'wsock32' needed when using MINGW on windows: https://github.com/CrowCpp/Crow/issues/759
find_package(Crow CONFIG REQUIRED)
target_link_libraries(track_mapper
        PRIVATE
            Crow::Crow asio::asio ws2_32 wsock32
)

# copy web app folder next to binary
# following https://jeremimucha.com/2021/05/cmake-managing-resources/
# TODO: fix unintended creation of res folder in build directory
set(webserver_files
        index.html
        main.js
        style.css
        res/github-mark.svg
)
add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/static/${webserver_files}

)
foreach(file IN LISTS webserver_files)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static/${webserver_files}
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_SOURCE_DIR}/data/web/${file}
                ${CMAKE_CURRENT_BINARY_DIR}/static/${file}
            COMMENT
                "Copying files for webserver ${CMAKE_CURRENT_BINARY_DIR}/static/${file}"
            APPEND
    )
endforeach()
add_custom_target(Webserver
        ALL DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/static/${webserver_files}
)
