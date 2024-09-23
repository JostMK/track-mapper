cmake_minimum_required(VERSION 3.28)
project(TrackMapper)

set(CMAKE_CXX_STANDARD 23)

set(FBX_SHARED 0)
set(FBX_STATIC_RTL 0) # MD version - same as ksEditor uses

# Taken from Samples CMakeLists.txt
if (NOT COMMAND fbx_target_finalize)
    INCLUDE("fbxsdk.cmake")
endif ()

if (FBX_SHARED)
    add_compile_definitions(FBXSDK_SHARED)
endif ()

add_library(TrackMapperSceneLib STATIC
        TrackScene.h
        TrackScene.cpp
        TrackCreator.cpp
        TrackCreator.h
)
target_link_libraries(TrackMapperSceneLib PRIVATE fbxsdk)
fbx_target_finalize(TrackMapperSceneLib)

add_executable(TrackMapperSceneConsoleApp
        main.cpp
)
target_link_libraries(TrackMapperSceneConsoleApp PRIVATE TrackMapperSceneLib)