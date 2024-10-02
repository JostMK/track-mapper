cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(GDAL 3.9 REQUIRED) # https://github.com/OSGeo/gdal
find_package(PROJ 9.4 REQUIRED CONFIG) # https://github.com/OSGeo/PROJ used by gdal

add_executable(GDALInfoTest
        main.cpp
)

target_link_libraries(GDALInfoTest TrackMapperMeshLib GDAL::GDAL)

# copies proj lib data files post build - sometimes errors occur if files not present (proj.db file)
add_custom_command(TARGET GDALInfoTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj"
        "$<TARGET_FILE_DIR:GDALInfoTest>/proj/"
        COMMENT "Copying proj data from $<TARGET_FILE_DIR:PROJ::proj>/..$<$<CONFIG:Debug>:/..>/share/proj to $<TARGET_FILE_DIR:GDALInfoTest>/proj/"
)
