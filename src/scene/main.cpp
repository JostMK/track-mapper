//
// Created by Jost on 05/09/2024.
//

#include <iostream>

#include "TrackScene.h"

using namespace TrackMapper::Scene;

SceneMesh createPlane(const Double3 &origin, const double size) {
    SceneMesh mesh(origin, 4, 2);

    const double halfSize = size * .5;
    constexpr Double3 UP{0, 1, 0};
    mesh.vertices.push_back({Double3{-halfSize, 0, -halfSize}, UP, {0, 0}});
    mesh.vertices.push_back({Double3{halfSize, 0, -halfSize}, UP, {1, 0}});
    mesh.vertices.push_back({Double3{-halfSize, 0, halfSize}, UP, {0, 1}});
    mesh.vertices.push_back({Double3{halfSize, 0, halfSize}, UP, {1, 1}});

    mesh.triangles.push_back(0);
    mesh.triangles.push_back(2);
    mesh.triangles.push_back(1);
    mesh.triangles.push_back(1);
    mesh.triangles.push_back(2);
    mesh.triangles.push_back(3);

    return mesh;
}

int main() {
    TrackScene scene;

    auto grassPlane = createPlane({0, 0, 0}, 100);
    scene.AddGrassMesh(grassPlane);

    auto roadPlane = createPlane({1, 0.1, 0}, 20);
    scene.AddRoadMesh(roadPlane);

    scene.AddSpawnPoint("AC_START_0", {0, 1, 0}, {0, 0, 1});
    scene.AddSpawnPoint("AC_PIT_0", {0, 1, 0}, {1, 0.1, 1});

    std::cout << "Enter fbx file output path: " << std::endl;
    std::string outputPath;
    std::cin >> outputPath;

    scene.Export(outputPath, true);
    scene.Export(outputPath.substr(0, outputPath.length()-4) + "-raw.fbx", false);

    return 0;
}
