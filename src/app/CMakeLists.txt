cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(PROJ 9.4 REQUIRED CONFIG) # needed for copying proj folder needed for TrackMapperMeshLib - dirty hack

add_executable(TrackMapperConsoleApp
        main.cpp
)
target_link_libraries(TrackMapperConsoleApp PRIVATE TrackMapperMeshLib TrackMapperSceneLib)

# copies proj lib data files post build needed right next to TrackMapperMeshLib - dirty hack
add_custom_command(TARGET TrackMapperConsoleApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
        "$<TARGET_FILE_DIR:TrackMapperConsoleApp>/proj/"
        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
)