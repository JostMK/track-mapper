cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

find_package(Crow CONFIG REQUIRED)

add_library(TrackMapperServerLib STATIC
        TestWebApp.cpp
        TestWebApp.h
)
target_link_libraries(TrackMapperServerLib PRIVATE TrackMapperGraphLib TrackMapperMeshLib Crow::Crow asio::asio)

if(WIN32)
    # see https://github.com/CrowCpp/Crow/issues/759
    target_link_libraries(TrackMapperServerLib INTERFACE ws2_32 wsock32)
endif()


add_executable(TrackMapperServerWebApp
        main.cpp
)
target_link_libraries(TrackMapperServerWebApp PRIVATE TrackMapperServerLib TrackMapperGraphLib)

# Target needs to be installed for static web files to be copied to the right location
install(TARGETS TrackMapperServerWebApp)
install(DIRECTORY static/ DESTINATION static/) # The destination has to be in the working directory
