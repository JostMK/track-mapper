//
// Created by Jost on 10/09/2024.
//

#include <iostream>

#include "../mesh/mesh_converter.h"
#include "../mesh/mesh_operations.h"
#include "../mesh/raster_reader.h"
#include "../scene/TrackScene.h"

using TrackScene = TrackMapper::Scene::TrackScene;
using Point3D = TrackMapper::Mesh::CGALPoint3;

void addTerrain(TrackScene &scene, Point3D &origin, bool originIsSet);
void addRoad(TrackScene &scene, const Point3D &origin, bool originIsSet);
void writeOut(const TrackScene &scene);

int main() {
    TrackScene scene;
    Point3D origin;
    bool originIsSet = false;

    bool quit = false;
    while (!quit) {
        std::cout << "Select option: add [T]errain, add [R]oad, [E]xport, [Q]uit:" << std::endl;
        std::string selection;
        std::cin >> selection;

        switch (std::tolower(selection[0])) {
            case 'q':
                quit = true;
                break;
            case 't':
                addTerrain(scene, origin, originIsSet);
                originIsSet = true;
                break;
            case 'r':
                addRoad(scene, origin, originIsSet);
                break;
            case 'e':
                writeOut(scene);
                quit = true;
                break;
            default:
                std::cout << "invalid option: " << selection[0] << std::endl;
                break;
        }

        std::cout << std::endl;
    }
}

void addTerrain(TrackScene &scene, Point3D &origin, const bool originIsSet) {
    std::cout << "Enter path to geo raster file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::cout << "Task 1/4: Opening geo dataset" << std::endl;
    TrackMapper::Raster::GDALDatasetWrapper dataset(inFilePath);
    const auto pointGrid = TrackMapper::Raster::readRasterData(dataset);

    std::cout << "Task 2/4: Creating mesh" << std::endl;
    auto mesh = TrackMapper::Mesh::meshFromRasterData(pointGrid);

    std::cout << "Task 3/4: Simplifying mesh" << std::endl;
    const double reductionRation = std::min(40e3 / mesh.number_of_vertices(), 0.5);
    TrackMapper::Mesh::reduceMesh(mesh, reductionRation);

    std::cout << "Task 4/4: Adding mesh to scene" << std::endl;
    if (!originIsSet) {
        origin = {pointGrid.origin.x, pointGrid.origin.y, pointGrid.origin.z};
    }
    auto meshOffset = Point3D{pointGrid.origin.x, pointGrid.origin.y, pointGrid.origin.z} - origin;
    auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {meshOffset.x(), meshOffset.y(), meshOffset.z()});

    scene.AddGrassMesh(sceneMesh);
    std::cout << "Finished: Added terrain with " << sceneMesh.vertices.size() << " vertices to scene" << std::endl;
}

void addRoad(TrackScene &scene, const Point3D &origin, const bool originIsSet) {
    if (!originIsSet) {
        std::cout << "Please first add at least one terrain befor adding any road" << std::endl;
        return;
    }

    // TODO implement
}

void writeOut(const TrackScene &scene) {
    std::cout << "Enter path for exported fbx file:" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    scene.Export(outFilePath);

    std::cout << "File successfully written to:\n" << outFilePath << std::endl;
}
