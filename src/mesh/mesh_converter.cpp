//
// Created by Jost on 10/09/2024.
//

#include <CGAL/Polygon_mesh_processing/compute_normal.h>

#include "mesh_converter.h"

namespace TrackMapper::Mesh {

    using SceneMesh = Scene::SceneMesh;

    SceneMesh cgalToSceneMesh(const CGALMesh &mesh, const CGALPoint3 &origin) {
        const int vertCount = static_cast<int>(mesh.number_of_vertices()); // should not exceed 40k
        const int trisCount = static_cast<int>(mesh.number_of_faces()) * 3;
        SceneMesh sceneMesh({origin.x(), origin.y(), origin.z()}, vertCount, trisCount);

        // add all vertices
        std::map<CGALMesh::Vertex_index, int> vertexMap;
        int vertexCount = 0;
        for (const auto vertex: mesh.vertices()) {
            auto point = mesh.point(vertex);
            vertexMap.insert(std::pair{vertex, vertexCount});
            Scene::Double3 position{point.x(), point.y(), point.z()};
            auto normal = CGAL::Polygon_mesh_processing::compute_vertex_normal(vertex, mesh);
            sceneMesh.vertices.emplace_back(position, Scene::Double3{normal.x(), normal.y(), normal.z()});
            vertexCount++;
        }

        // add all triangles
        for (const auto face: mesh.faces()) {
            // to get the vertices of a face one needs to iterate over all the halfedges of the face
            // and get their target
            for (const auto hi: halfedges_around_face(mesh.halfedge(face), mesh)) {
                // should never be more than 3 vertices per face since simplification works on triangulated meshes
                auto vertex = CGAL::target(hi, mesh);
                int triangleIndex = vertexMap.at(vertex);
                sceneMesh.triangles.push_back(triangleIndex);
            }
        }

        return sceneMesh;
    }
} // namespace TrackMapper::Mesh
