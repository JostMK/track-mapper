cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(CGAL REQUIRED) # https://github.com/CGAL/cgal
find_package(GDAL REQUIRED) # https://github.com/OSGeo/gdal

find_package(PROJ REQUIRED CONFIG) # https://github.com/OSGeo/PROJ used by gdal

add_library(TrackMapperMeshLib STATIC
        raster_reader.h
        raster_reader.cpp
        mesh_operations.h
        mesh_operations.cpp
)
target_link_libraries(TrackMapperMeshLib CGAL::CGAL GDAL::GDAL)
# copies proj lib data files post build - sometimes error if not there
add_custom_command(TARGET TrackMapperMeshLib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:PROJ::proj>/../share/proj"
        "$<TARGET_FILE_DIR:TrackMapperMeshLib>/proj/"
        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/../share/proj"
)


add_executable(TrackMapperMeshConsoleApp
        main.cpp
)
target_link_libraries(TrackMapperMeshConsoleApp PRIVATE TrackMapperMeshLib)