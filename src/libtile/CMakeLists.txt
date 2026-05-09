cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(CGAL 5.6 REQUIRED) # https://github.com/CGAL/cgal
find_package(GDAL 3.9 REQUIRED) # https://github.com/OSGeo/gdal

find_package(PROJ 9.4 REQUIRED CONFIG) # https://github.com/OSGeo/PROJ used by gdal

add_library(LibTile STATIC
        GeoTile.h
        GeoTile.cpp
)

target_link_libraries(LibTile CGAL::CGAL GDAL::GDAL)

add_executable(TESTTEST
    main.cpp
)
target_link_libraries(TESTTEST PRIVATE LibTile LibScene)

# copies proj lib data files post build - sometimes errors occur if files not present (proj.db file)
#add_custom_command(TARGET TESTTEST POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "$<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
#        "$<TARGET_FILE_DIR:LibTile>/proj/"
#        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj to $<TARGET_FILE_DIR:LibTile>/proj/"
#)