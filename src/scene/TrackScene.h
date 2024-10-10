//
// Created by Jost on 05/09/2024.
//

#ifndef TRACKSCENE_H
#define TRACKSCENE_H

#include <string>
#include <utility>
#include <vector>

namespace TrackMapper::Scene {
    struct Double3 {
        union {
            struct {
                double x, y, z;
            };
            struct {
                double r, g, b;
            };
        };
    };

    struct Double2 {
        union {
            struct {
                double x, y;
            };
            struct {
                double u, v;
            };
        };
    };

    struct Vertex {
        Double3 position;
        Double3 normal;
        Double2 uv;
    };

    struct SceneMesh {
        std::string name;
        Double3 origin;
        Double3 forward;
        std::vector<Vertex> vertices;
        std::vector<int> triangles;

        /**
         * Creates an mesh with set vertex and triangle count
         * @param origin the origin of the mesh in 3d space
         * @param vertexCount the reserved size for vertices
         * @param triangleCount the reserved size for triangle indices
         */
        SceneMesh(const Double3 &origin, const int vertexCount, const int triangleCount) : origin(origin), forward{0,0,1} {
            vertices.reserve(vertexCount);
            triangles.reserve(triangleCount);
        }

        /**
         * Creates an empty mesh with facing a specific direction - used for specifying the car spawn point
         * @param name the name of the spawn point - should follow ksEditor naming convention
         * @param origin the origin of the mesh in 3d space
         * @param forward the direction the local z axis should point
         */
        SceneMesh(std::string name, const Double3 &origin, const Double3 &forward) : name(std::move(name)), origin(origin), forward{forward} {}
    };

    class TrackScene {
    public:
        void AddGrassMesh(SceneMesh &mesh, bool hasCollision = false);

        void AddRoadMesh(SceneMesh &mesh);

        /// @note name should follow the naming convention for spawn points used by ksEditor
        void AddSpawnPoint(const std::string& name, const Double3 &position, const Double3 &direction);

        /// @note ksEditor cannot read FBX SDK 2020 binaries - export with asciiFormat set to true
        /// @note blender can only read binary fbx files - export with asciiFormat set to false
        void Export(const std::string &filePath, bool asciiFormat = true) const;

    private:
        std::vector<SceneMesh> mGrassMeshes;
        std::vector<SceneMesh> mRoadMeshes;
        std::vector<SceneMesh> mEmptyMeshes;

        int mGrassCounter = 0;
        int mPhysicsCounter = 1; // objects with a physics id greater 0 have collisions
    };
} // namespace TrackMapper::Scene

#endif // TRACKSCENE_H
