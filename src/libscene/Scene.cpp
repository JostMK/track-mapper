//
// Created by Jost on 31/12/2025.
//

#include "Scene.h"

#include <array>
#include <utility>

namespace LibScene {
    void Scene::add_mesh(const Mesh &mesh) { m_meshes.push_back(mesh); }

    Error Scene::export_to_gltf(const std::string &filepath) const {
        struct GLTFBuffer {
            std::string uri;
            int byteLength;
        };

        struct GLTFBufferView {
            int buffer;
            int byteLength;
        };

        struct GLTFAccessor {
            int bufferView{};
            int componentType = 5125; // UNSIGNED_INT
            int count = 1;
            std::string type = "SCALAR";

            GLTFAccessor() = default;
            GLTFAccessor(const int bufferViewIndex, const int count) : bufferView(bufferViewIndex), count(count) {}
            GLTFAccessor(const int bufferViewIndex, const int componentType, const int count, std::string type) :
                bufferView(bufferViewIndex), componentType(componentType), count(count), type(std::move(type)) {}
        };

        struct GLTFPositionAccessor : GLTFAccessor {
            std::array<float, 3> max{};
            std::array<float, 3> min{};

            GLTFPositionAccessor() : GLTFAccessor(0, 5126 /*FLOAT*/, 0, "VEC3") {}
            GLTFPositionAccessor(const int bufferViewIndex, const int count, const std::array<float, 3> &max,
                                 const std::array<float, 3> &min) :
                GLTFAccessor(bufferViewIndex, 5126 /*FLOAT*/, count, "VEC3"), max(max), min(min) {}
        };

        struct GLTFMeshAttribute {
            int POSITION;
        };

        struct GLTFMeshPrimitive {
            std::vector<GLTFMeshAttribute> attributes;
            int indices;
        };

        struct GLTFMesh {
            std::vector<GLTFMeshPrimitive> primitives;
        };

        struct GLTFNode {
            std::string name;
            std::array<float, 3> translation{};
            int mesh{};
        };

        struct GLTFScene {
            std::vector<int> nodes;
        };

        struct GLTFFile {
            int scene = 0;
            std::vector<GLTFScene> scenes;
            std::vector<GLTFNode> nodes;
            std::vector<GLTFMesh> meshes;
            std::vector<GLTFBuffer> buffers;
            std::vector<GLTFBufferView> bufferViews;
            std::vector<GLTFAccessor> accessor;
        };


        // Creating gltf file structure:
        GLTFFile gltf{};

        GLTFScene scene;
        for (const auto &mesh: m_meshes) {
            GLTFNode gltfNode{
                    "Mesh" + scene.nodes.size(),
                    {mesh.origin.x, mesh.origin.y, mesh.origin.z},
                    static_cast<int>(gltf.meshes.size()),
            };
            scene.nodes.push_back(static_cast<int>(gltf.nodes.size()));

            const int accessor_count = static_cast<int>(gltf.accessor.size());
            GLTFMesh gltfMesh{{GLTFMeshPrimitive{{GLTFMeshAttribute{accessor_count}}, accessor_count + 1}}};

            const int buffer_count = static_cast<int>(gltf.buffers.size());
            GLTFPositionAccessor gltfPositionsAccessor{
                    buffer_count, static_cast<int>(mesh.vertices.size() * 3), {0, 0, 0}, {0, 0, 0}};
            GLTFAccessor gltfIndicesAccessor{buffer_count + 1, static_cast<int>(mesh.faces.size() * 3)};

            gltf.nodes.push_back(gltfNode);
            gltf.meshes.push_back(gltfMesh);
            gltf.accessor.push_back(gltfPositionsAccessor);
            gltf.accessor.push_back(gltfIndicesAccessor);
        }

        gltf.scenes.push_back(scene);

        return Error::NONE;
    }
} // namespace LibScene
