cmake_minimum_required(VERSION 3.28)
project(CGALTesting)

set(CMAKE_CXX_STANDARD 23)

set(FBX_SHARED 0)
set(FBX_STATIC_RTL 0) # MD version - same as ksEditor uses

# Taken from Samples CMakeLists.txt
if(NOT COMMAND fbx_target_finalize)
    INCLUDE("fbxsdk.cmake")
endif()

if(FBX_SHARED)
    add_compile_definitions(FBXSDK_SHARED)
endif()


add_executable(FBXPrintTest printfbx.cpp)
target_link_libraries(FBXPrintTest PRIVATE fbxsdk)
fbx_target_finalize(FBXPrintTest)


add_executable(FBXExportTest exportfbx.cpp)
target_link_libraries(FBXExportTest PRIVATE fbxsdk)
fbx_target_finalize(FBXExportTest)

add_executable(FBXConverterTest convertfbx.cpp)
target_link_libraries(FBXConverterTest PRIVATE fbxsdk)
fbx_target_finalize(FBXConverterTest)