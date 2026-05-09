//
// Created by Jost on 31/12/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "Mesh.h"

namespace LibScene {

    class Scene {
    public:
        Scene() = default;

        void add_mesh(const Mesh &mesh);

        Error export_to_gltf(const std::string &filepath) const;

    private:
        std::vector<Mesh> m_meshes;
    };

} // namespace LibScene

#endif // SCENE_H
