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

    namespace SMS = CGAL::Surface_mesh_simplification;

    Mesh meshFromRasterData(const Raster::PointGrid &point_grid) {
        Mesh mesh;
        std::vector<Mesh::Vertex_index> vertex_indices;
        vertex_indices.reserve(point_grid.points.size());

        const auto quads = (point_grid.sizeX - 1) * (point_grid.sizeY - 1);
        mesh.reserve(point_grid.points.size(),
                     // one triangle per quad + all the right most vertical edges + all the lowest horizontal edges
                     quads * 3 + (point_grid.sizeX - 1) + (point_grid.sizeY - 1), quads * 2);

        for (auto [x, y, z] : point_grid.points) {
            const auto vertexIndex = mesh.add_vertex({x, y, z});
            vertex_indices.push_back(vertexIndex);
        }

        for (int y = 0; y < point_grid.sizeY - 1; ++y) {
            for (int x = 0; x < point_grid.sizeX - 1; ++x) {
                const int indexTL = y * point_grid.sizeX + x;
                const int indexTR = y * point_grid.sizeX + (x + 1);
                const int indexBL = (y + 1) * point_grid.sizeX + x;
                const int indexBR = (y + 1) * point_grid.sizeX + (x + 1);
                // TODO: fix normals by inverting winding order, inverted because image extends downwards invers to 3d y-axis
                mesh.add_face(vertex_indices[indexTL], vertex_indices[indexBL], vertex_indices[indexTR]);
                mesh.add_face(vertex_indices[indexTR], vertex_indices[indexBL], vertex_indices[indexBR]);
            }
        }

        return mesh;
    }

    Mesh meshFromPath(const Path &path, const double width) {
    /*
        Mesh mesh;
        std::vector<Mesh::Vertex_index> vertex_indices;
        vertex_indices.reserve((path.points.size()-1) * 2);

        const auto quads = path.points.size() - 2;
        mesh.reserve(vertex_indices.size(), 4 * quads + 1, 2 * quads);

        for (auto i = 0; i < path.points.size()-1; ++i) {
            const auto [x1, y1] = path.points[i];
            const auto [x2, y2] = path.points[i + 1];

            // normalized direction from point 1 to point 2 rotated 90 degree
            const Eigen::Vector2d dir = Eigen::Vector2d(y1 - y2, x2 - x1).normalized();
            const Eigen::Vector2d mid((x2 - x1) / 2, (y2 - y1) / 2);
            const Eigen::Vector2d left = mid - dir * width / 2;
            const Eigen::Vector2d right = mid + dir * width / 2;

            const auto vertexIndexL = mesh.add_vertex({left.x(), 0, left.y()});
            vertex_indices.push_back(vertexIndexL);

            const auto vertexIndexR = mesh.add_vertex({right.x(), 0, right.y()});
            vertex_indices.push_back(vertexIndexR);
        }

        for (auto i = 0; i < quads; ++i) {
            const int indexL1 = 2 * i + 0;
            const int indexR1 = 2 * i + 1;
            const int indexL2 = 2 * i + 2;
            const int indexR2 = 2 * i + 3;
            mesh.add_face(vertex_indices[indexL1], vertex_indices[indexR1], vertex_indices[indexL2]);
            mesh.add_face(vertex_indices[indexL2], vertex_indices[indexR1], vertex_indices[indexR2]);
        }

        return mesh;

        */

        Mesh mesh;

        for (auto i = 0; i < path.points.size()-1; ++i) {
            const auto [x1, y1] = path.points[i];
            const auto [x2, y2] = path.points[i + 1];

            const auto vertexIndex1 = mesh.add_vertex({x1, 0, y1});
            const auto vertexIndex2 = mesh.add_vertex({x2, 0, y2});
            mesh.add_edge(vertexIndex1, vertexIndex2);
        }

        return mesh;
    }

    int reduceMesh(Mesh &mesh, const double reduction_ratio) {
        typedef SMS::GarlandHeckbert_plane_policies<Mesh, Kernel> Classic_plane;
        typedef Classic_plane::Get_cost GH_cost;
        typedef Classic_plane::Get_placement GH_placement;
        typedef SMS::Bounded_normal_change_placement<GH_placement> Bounded_GH_placement;

        const Classic_plane gh_policies(mesh);
        const GH_cost &gh_cost = gh_policies.get_cost();
        const GH_placement &gh_placement = gh_policies.get_placement();
        const Bounded_GH_placement placement(gh_placement);

        const SMS::Edge_count_ratio_stop_predicate<Mesh> stop(reduction_ratio);

        const int edgesRemoved =
                SMS::edge_collapse(mesh, stop, CGAL::parameters::get_cost(gh_cost).get_placement(placement));

        return edgesRemoved;
    }

    void writeMeshToFile(const Mesh &mesh, const std::string &filepath) {
        CGAL::IO::write_polygon_mesh(filepath, mesh, CGAL::parameters::stream_precision(17));
    }
} // namespace TrackMapper::Mesh
