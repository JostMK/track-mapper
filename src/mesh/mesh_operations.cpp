//
// Created by Jost on 09/07/2024.
//

#include <CGAL/Simple_cartesian.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/GarlandHeckbert_policies.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

#include "mesh_operations.h"

namespace TrackMapper::Mesh {

    using CGALVector3 = CGALKernel::Vector_3;

    namespace SMS = CGAL::Surface_mesh_simplification;

    CGALMesh meshFromRasterData(const Raster::PointGrid &point_grid) {
        CGALMesh mesh;
        std::vector<CGALMesh::Vertex_index> vertex_indices;
        vertex_indices.reserve(point_grid.points.size());

        const auto quads = (point_grid.sizeX - 1) * (point_grid.sizeY - 1);
        mesh.reserve(point_grid.points.size(),
                     // one triangle per quad + all the right most vertical edges + all the lowest horizontal edges
                     quads * 3 + (point_grid.sizeX - 1) + (point_grid.sizeY - 1), quads * 2);

        for (auto [x, y, z]: point_grid.points) {
            const auto vertexIndex = mesh.add_vertex({x, y, z});
            vertex_indices.push_back(vertexIndex);
        }

        for (int y = 0; y < point_grid.sizeY - 1; ++y) {
            for (int x = 0; x < point_grid.sizeX - 1; ++x) {
                // Note: points extend in positive x and positive z axis
                // Note: needs to conform with the right handed coordinate system of fbx
                const int indexTL = y * point_grid.sizeX + x;
                const int indexTR = y * point_grid.sizeX + (x + 1);
                const int indexBL = (y + 1) * point_grid.sizeX + x;
                const int indexBR = (y + 1) * point_grid.sizeX + (x + 1);
                mesh.add_face(vertex_indices[indexTL], vertex_indices[indexTR], vertex_indices[indexBL]);
                mesh.add_face(vertex_indices[indexTR], vertex_indices[indexBR], vertex_indices[indexBL]);
            }
        }

        return mesh;
    }

    /// @note path need at least 4 points
    /// @note subdivision has to be greater or equal to 2
    CGALMesh meshFromPath(const Path &path, const double width, const int subdivisions) {
        const int segmentCount = subdivisions - 1;
        const double segmentWidth = width / segmentCount;

        if(path.points.size() < 4) {
            // Todo: maybe log error / warning
            return {};
        }

        CGALMesh mesh;
        std::vector<CGALMesh::Vertex_index> vertex_indices;
        vertex_indices.reserve((path.points.size() - 2) * subdivisions);

        const int quads = static_cast<int>(path.points.size() - 3) * segmentCount;
        mesh.reserve(vertex_indices.capacity(), quads * 3 + (path.points.size() - 3) + segmentCount, 2 * quads);

        for (auto i = 1; i < path.points.size() - 1; ++i) {
            // TODO: fix division by zero when consecutive points share same position
            // fix: do calculation without y component to avoid slanted halfway vector
            // TODO: Add slight slanting in corners to create on-camber corners
            const CGALPoint3 p1{path.points[i - 1].x(), 0, path.points[i - 1].z()};
            const CGALPoint3 p2{path.points[i].x(), 0, path.points[i].z()};
            const CGALPoint3 p3{path.points[i + 1].x(), 0, path.points[i + 1].z()};
            const CGALVector3 v1 = p1 - p2;
            const CGALVector3 v2 = p3 - p2;

            const CGALVector3 n1 = v1 / std::sqrt(v1.squared_length());
            const CGALVector3 n2 = v2 / std::sqrt(v2.squared_length());
            const CGALVector3 h = n1 + n2;
            const CGALVector3 nh = h / std::sqrt(h.squared_length());

            // tests if v2 points to the left of v1 relative to the horizontal plane
            // flip order of adding vertices to assure the vertices to the left always gets pushed first
            const int iterationSign =
                    CGAL::scalar_product(CGAL::cross_product(v2, v1), CGALVector3(0, 1, 0)) > 0 ? 1 : -1;

            // adds vertices from left to right
            for (auto j = 0; j < subdivisions; j++) {
                // TODO: handle cases where extend overlaps with the extend of the previous point
                //      -> merge vertices in the middle -> problems with creating faces later
                const double extend = -width * 0.5 + j * segmentWidth;
                const auto vertexIndex = mesh.add_vertex(path.points[i] + nh * iterationSign * extend);
                vertex_indices.push_back(vertexIndex);
            }
        }

        for (auto i = 0; i < path.points.size() - 3; ++i) {
            for (int j = 0; j < segmentCount; j++) {
                const int indexL1 = i * subdivisions + j;
                const int indexR1 = i * subdivisions + j + 1;
                const int indexL2 = (i + 1) * subdivisions + j;
                const int indexR2 = (i + 1) * subdivisions + j + 1;
                // NOTE: to conform with ksEditor the winding order has to be clockwise
                mesh.add_face(vertex_indices[indexL1], vertex_indices[indexL2], vertex_indices[indexR1]);
                mesh.add_face(vertex_indices[indexR1], vertex_indices[indexL2], vertex_indices[indexR2]);
            }
        }

        return mesh;
    }

    int reduceMesh(CGALMesh &mesh, const double reduction_ratio) {
        typedef SMS::GarlandHeckbert_plane_policies<CGALMesh, CGALKernel> Classic_plane;
        typedef Classic_plane::Get_cost GH_cost;
        typedef Classic_plane::Get_placement GH_placement;
        typedef SMS::Bounded_normal_change_placement<GH_placement> Bounded_GH_placement;

        const Classic_plane gh_policies(mesh);
        const GH_cost &gh_cost = gh_policies.get_cost();
        const GH_placement &gh_placement = gh_policies.get_placement();
        const Bounded_GH_placement placement(gh_placement);

        const SMS::Edge_count_ratio_stop_predicate<CGALMesh> stop(reduction_ratio);

        const int edgesRemoved =
                SMS::edge_collapse(mesh, stop, CGAL::parameters::get_cost(gh_cost).get_placement(placement));

        return edgesRemoved;
    }

    void writeMeshToFile(const CGALMesh &mesh, const std::string &filepath) {
        CGAL::IO::write_polygon_mesh(filepath, mesh, CGAL::parameters::stream_precision(17));
    }
} // namespace TrackMapper::Mesh
