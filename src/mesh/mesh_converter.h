//
// Created by Jost on 10/09/2024.
//

#ifndef MESH_CONVERTER_H
#define MESH_CONVERTER_H

#include "../scene/TrackScene.h"
#include "mesh_operations.h"

namespace TrackMapper::Mesh {
    Scene::SceneMesh cgalToSceneMesh(const CGALMesh &mesh, const CGALPoint3 &origin, const std::function<Scene::Double2(int)>& uvEvaluator = [](int _) -> Scene::Double2{return {};});
}

#endif // MESH_CONVERTER_H
