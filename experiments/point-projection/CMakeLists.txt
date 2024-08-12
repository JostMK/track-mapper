cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(CGAL 5.6 REQUIRED) # https://github.com/CGAL/cgal
find_package(GDAL 3.9 REQUIRED) # https://github.com/OSGeo/gdal

find_package(PROJ 9.4 REQUIRED CONFIG) # https://github.com/OSGeo/PROJ used by gdal


add_executable(GDALProjectionTest
        main.cpp
)

target_link_libraries(GDALProjectionTest CGAL::CGAL GDAL::GDAL)
# copies proj lib data files post build - sometimes error if not there
add_custom_command(TARGET GDALProjectionTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
        "$<TARGET_FILE_DIR:GDALProjectionTest>/proj/"
        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
)