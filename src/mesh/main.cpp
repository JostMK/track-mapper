//
// Created by Jost on 09/07/2024.
//

#include <chrono>
#include <iostream>

#include "mesh_operations.h"
#include "raster_reader.h"

void createPath();
void createMesh();

int main() {
    bool quit = false;
    while (!quit) {
        std::cout << "Select option: create mesh from raster (m), create path in raster space (p), quit (q):" << std::endl;
        std::string selection;
        std::cin >> selection;

        switch (selection[0]) {
            case 'q':
                quit = true;
                break;
            case 'm':
                createMesh();
                break;
            case 'p':
                createPath();
                break;
            default:
                std::cout << "invalid option: " << selection[0] << std::endl;
                break;
        }

        std::cout << std::endl;
    }
}

void createPath() {

    std::cout << "Enter Path to path csv file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::cout << "Enter Path to geo raster file (for projection reference):" << std::endl;
    std::string inRasterPath;
    std::cin >> inRasterPath;

    std::vector<TrackMapper::Raster::Point> points;

    if (std::ifstream pathFile(inFilePath); pathFile.is_open()) {
        std::string line;
        while (pathFile.good()) {
            std::getline(pathFile, line);
            const size_t seperator = line.find_first_of(';');
            const double lat = std::stod(line.substr(0, seperator));
            const double lng = std::stod(line.substr(seperator + 1));
            points.emplace_back(lat, lng, 0);
        }
        pathFile.close();
    }

    TrackMapper::Raster::reprojectPointsIntoRaster(inRasterPath, points);

    TrackMapper::Mesh::Path path;
    path.points.reserve(points.size());
    for (auto [x, y, z]: points) {
        path.points.emplace_back(x, y);
    }

    const auto mesh = TrackMapper::Mesh::meshFromPath(path, 6);

    std::cout << "\nSpecify file output path: (.off .obj .stl .ply .ts .vtp)" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    TrackMapper::Mesh::writeMeshToFile(mesh, outFilePath);
}

void createMesh() {
    std::cout << "Enter Path to geo raster file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    // ReSharper disable CppJoinDeclarationAndAssignment
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    // ReSharper activate CppJoinDeclarationAndAssignment

    std::cout << "Reading data from file.." << std::endl;
    start_time = std::chrono::steady_clock::now();

    const auto data = TrackMapper::Raster::readRasterData(inFilePath);
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
    std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges()
              << " Faces: " << mesh.number_of_faces() << std::endl;

    std::cout << "Simplify mesh? (y): " << std::endl;
    std::string shouldSimplify;
    std::cin >> shouldSimplify;

    if (shouldSimplify.starts_with('y')) {
        std::cout << "Reduction ratio: " << std::endl;
        double reductionRation;
        std::cin >> reductionRation;

        std::cout << "Simplifying mesh.." << std::endl;
        start_time = std::chrono::steady_clock::now();

        const int edgesRemoved = TrackMapper::Mesh::reduceMesh(mesh, reductionRation);

        end_time = std::chrono::steady_clock::now();
        std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                  << "ms" << std::endl;
        std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges()
                  << " Faces: " << mesh.number_of_faces() << std::endl;
        std::cout << "Edges removed: " << edgesRemoved << std::endl;
    }

    std::cout << "\nSpecify file output path: (.off .obj .stl .ply .ts .vtp)" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    TrackMapper::Mesh::writeMeshToFile(mesh, outFilePath);
}
