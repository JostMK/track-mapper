cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(Crow CONFIG REQUIRED)
find_package(PROJ 9.4 REQUIRED CONFIG) # needed for copying proj folder needed for TrackMapperMeshLib - dirty hack

add_library(TrackMapperServerLib STATIC
        BasicWebApp.h
        BasicWebApp.cpp
        errors.h
        TrackData.h
)
target_link_libraries(TrackMapperServerLib PRIVATE TrackMapperGraphLib TrackMapperMeshLib Crow::Crow asio::asio)

if (WIN32)
    # see https://github.com/CrowCpp/Crow/issues/759
    target_link_libraries(TrackMapperServerLib INTERFACE ws2_32 wsock32)
endif ()

# copies static web files post build needed right next to TrackMapperServerLib - dirty hack
add_custom_command(TARGET TrackMapperServerLib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/static/"
        "$<TARGET_FILE_DIR:TrackMapperServerLib>/static/"
        COMMENT "Copying static web files from ${CMAKE_CURRENT_SOURCE_DIR}/static/"
)

# copies proj lib data files post build needed right next to TrackMapperMeshLib - dirty hack
add_custom_command(TARGET TrackMapperServerLib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
        "$<TARGET_FILE_DIR:TrackMapperServerLib>/proj/"
        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
)


add_executable(TrackMapperServerWebApp
        main.cpp
)
target_link_libraries(TrackMapperServerWebApp PRIVATE TrackMapperServerLib)