//
// Created by Jost on 09/07/2024.
//

#include <chrono>
#include <iostream>

#include "gdal_wrapper.h"
#include "mesh_operations.h"
#include "raster_reader.h"

void createPath();
void createMesh();

int main() {
    bool quit = false;
    while (!quit) {
        std::cout << "Select option: create mesh from raster (m), create path in raster space (p), quit (q):"
                  << std::endl;
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
    TrackMapper::Raster::GDALDatasetWrapper dataset(inRasterPath);
    const auto grid = TrackMapper::Raster::readRasterData(dataset);

    std::vector<TrackMapper::Raster::OSMPoint> points;
    if (std::ifstream pathFile(inFilePath); pathFile.is_open()) {
        std::string line;
        while (pathFile.good()) {
            std::getline(pathFile, line);
            const size_t seperator = line.find_first_of(';');
            const double lat = std::stod(line.substr(0, seperator));
            const double lng = std::stod(line.substr(seperator + 1));
            points.emplace_back(lat, lng);
        }
        pathFile.close();
    }

    TrackMapper::Raster::ProjectionWrapper dstProjRef = grid.projRef;

    while (!dstProjRef.IsValid()) {
        std::cout << "No valid projection reference in provided raster file!" << std::endl;
        std::cout << "Specify projection reference manually (or press 'q' to quit):" << std::endl;
        std::string inProjRef;
        std::getline(std::cin >> std::ws, inProjRef);

        if (inProjRef[0] == 'q')
            return;

        dstProjRef = TrackMapper::Raster::ProjectionWrapper(inProjRef);
    }

    TrackMapper::Raster::reprojectOSMPoints(points, dstProjRef);

    TrackMapper::Mesh::Path path;
    path.points.reserve(points.size());
    for (auto [x, y]: points) {
        TrackMapper::Raster::Point p{x - grid.origin.x, 0, y - grid.origin.z};
        TrackMapper::Raster::SetHeightFromGrid(grid, p);
        path.points.emplace_back(p.x, p.y, -p.z); // fbx coordinate system needs z mirroring
    }

    const auto mesh = TrackMapper::Mesh::meshFromPath(path, 6, 5);

    std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges()
              << " Faces: " << mesh.number_of_faces() << std::endl;

    std::cout << "\nSpecify file output path: (.off .obj .stl .ply .ts .vtp)" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    TrackMapper::Mesh::writeMeshToFile(mesh, outFilePath);
}

void createMesh() {
    std::cout << "Enter Path to geo raster file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::cout << "\nSpecify file output path: (.off .obj .stl .ply .ts .vtp)" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    bool split_mesh;
    {
        std::cout << "Split mesh? (y): " << std::endl;
        std::string shouldSplit;
        std::cin >> shouldSplit;
        split_mesh = shouldSplit.starts_with('y');
    }

    // ReSharper disable CppJoinDeclarationAndAssignment
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    // ReSharper activate CppJoinDeclarationAndAssignment

    std::cout << "Reading data from file.." << std::endl;
    start_time = std::chrono::steady_clock::now();

    TrackMapper::Raster::GDALDatasetWrapper dataset(inFilePath);
    const auto data = TrackMapper::Raster::readRasterData(dataset);
    end_time = std::chrono::steady_clock::now();
    std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
              << std::endl;
    std::cout << "Size: x: " << data.sizeX << " y: " << data.sizeY << " Points: " << data.points.size() << std::endl;

    if (split_mesh) {
        constexpr int MAX_CHUNK_VERTEX_COUNT = 40000;
        const int vertex_count = dataset.GetSizeX() * dataset.GetSizeY();
        const int chunk_count = vertex_count / MAX_CHUNK_VERTEX_COUNT + 1;
        const int vertex_per_chunk_side =
                static_cast<int>(std::ceil(std::sqrt(vertex_count / static_cast<double>(chunk_count))));
        const int chunk_count_x = dataset.GetSizeX() / vertex_per_chunk_side + 1;
        const int chunk_count_y = dataset.GetSizeY() / vertex_per_chunk_side + 1;

        std::cout << "Splitting into " << chunk_count << " chunks" << std::endl;

        std::vector<TrackMapper::Raster::PointGrid> chunks;
        chunks.resize(chunk_count_x * chunk_count_y);

        for (int cY = 0; cY < chunk_count_y; ++cY) {
            for (int cX = 0; cX < chunk_count_x; ++cX) {
                auto &[points, sizeX, sizeY, pixelSizeX, pixelSizeY, origin, projRef, transform] =
                        chunks[cY * chunk_count_x + cX];

                sizeX = cX == chunk_count_x - 1 ? data.sizeX % vertex_per_chunk_side : vertex_per_chunk_side + 1;
                sizeY = cY == chunk_count_y - 1 ? data.sizeY % vertex_per_chunk_side : vertex_per_chunk_side + 1;
                pixelSizeX = data.pixelSizeX;
                pixelSizeY = data.pixelSizeY;
                origin = data.origin + TrackMapper::Raster::Point{cX * vertex_per_chunk_side * pixelSizeX, 0,
                                                                  cY * vertex_per_chunk_side * pixelSizeY};
                transform = data.transform;
                projRef = data.projRef;

                points.resize(sizeX * sizeY);
                for (int pY = 0; pY < sizeY; ++pY) {
                    for (int pX = 0; pX < sizeX; ++pX) {
                        points[pY * sizeX + pX] = data.points[(cY * vertex_per_chunk_side + pY) * data.sizeX +
                                                              cX * vertex_per_chunk_side + pX];
                    }
                }
            }
        }

        std::string out_path_prefix = outFilePath.substr(0, outFilePath.size() - 4).append("-");
        std::string out_path_suffix = outFilePath.substr(outFilePath.size() - 4);
        int index = 1;
        for (const auto &chunk: chunks) {
            std::cout << "Creating chunk " << index << std::endl;
            start_time = std::chrono::steady_clock::now();

            auto mesh = TrackMapper::Mesh::meshFromRasterData(chunk);

            end_time = std::chrono::steady_clock::now();
            std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                      << "ms" << std::endl;

            TrackMapper::Mesh::writeMeshToFile(mesh, out_path_prefix + std::to_string(index).append(out_path_suffix));
            ++index;
        }
    } else {
        std::cout << "Creating mesh.." << std::endl;
        start_time = std::chrono::steady_clock::now();

        auto mesh = TrackMapper::Mesh::meshFromRasterData(data);

        end_time = std::chrono::steady_clock::now();
        std::cout << "..in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                  << "ms" << std::endl;
        std::cout << "Vertices: " << mesh.number_of_vertices() << " Edges: " << mesh.number_of_edges()
                  << " Faces: " << mesh.number_of_faces() << std::endl;

        std::cout << "Writing mesh to disk.." << std::endl;
        TrackMapper::Mesh::writeMeshToFile(mesh, outFilePath);
    }
}
