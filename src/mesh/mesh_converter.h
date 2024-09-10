//
// Created by Jost on 10/09/2024.
//

#ifndef MESH_CONVERTER_H
#define MESH_CONVERTER_H

#include "../scene/TrackScene.h"
#include "mesh_operations.h"

namespace TrackMapper::Mesh {
    Scene::SceneMesh cgalToSceneMesh(const CGALMesh &mesh, const CGALPoint3 &origin);
}

#endif // MESH_CONVERTER_H
