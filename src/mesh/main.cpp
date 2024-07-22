//
// Created by Jost on 09/07/2024.
//

#include <chrono>
#include <iostream>

#include "raster_reader.h"
#include "mesh_operations.h"

int main() {
    std::cout << "Enter Path to geo raster file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    // ReSharper disable CppJoinDeclarationAndAssignment
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    // ReSharper activate CppJoinDeclarationAndAssignment

    std::cout << "Reading data from file.." << std::endl;
    start_time = std::chrono::steady_clock::now();

    const auto data = TrackMapper::Mesh::readRasterData(inFilePath);
    end_time = std::chrono::steady_clock::now();
    std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
            << std::endl;
    std::cout << "Size: x: " << data.sizeX << " y: " << data.sizeY << " Points: " << data.points.size() << std::endl;


    std::cout << "Creating mesh.." << std::endl;
    start_time = std::chrono::steady_clock::now();

    auto mesh = TrackMapper::Mesh::meshFromRasterData(data);

    end_time = std::chrono::steady_clock::now();
    std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
            << std::endl;
    std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges() << " Faces: " << mesh
            .number_of_faces() << std::endl;

    std::cout << "Simplify mesh? (y): " << std::endl;
    std::string shouldSimplify;
    std::cin >> shouldSimplify;

    if(shouldSimplify.starts_with('y')) {
        std::cout << "Simplifying mesh.." << std::endl;
        start_time = std::chrono::steady_clock::now();

        const int edgesRemoved = TrackMapper::Mesh::reduceMesh(mesh, 0.1);

        end_time = std::chrono::steady_clock::now();
        std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
                << std::endl;
        std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges() << " Faces: " << mesh
                .number_of_faces() << std::endl;
        std::cout << "Edges removed: " << edgesRemoved << std::endl;
    }

    std::cout << "\nSpecify file output path: (.off .obj .stl .ply .ts .vtp)" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    TrackMapper::Mesh::writeMeshToFile(mesh, outFilePath);
}
