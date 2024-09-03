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

    using Vector3 = Kernel::Vector_3;

    namespace SMS = CGAL::Surface_mesh_simplification;

    Mesh meshFromRasterData(const Raster::PointGrid &point_grid) {
        Mesh mesh;
        std::vector<Mesh::Vertex_index> vertex_indices;
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
                const int indexTL = y * point_grid.sizeX + x;
                const int indexTR = y * point_grid.sizeX + (x + 1);
                const int indexBL = (y + 1) * point_grid.sizeX + x;
                const int indexBR = (y + 1) * point_grid.sizeX + (x + 1);
                // TODO: fix normals by inverting winding order, inverted because image extends downwards invers to 3d
                // y-axis
                mesh.add_face(vertex_indices[indexTL], vertex_indices[indexBL], vertex_indices[indexTR]);
                mesh.add_face(vertex_indices[indexTR], vertex_indices[indexBL], vertex_indices[indexBR]);
            }
        }

        return mesh;
    }

    Mesh meshFromPath(const Path &path, const double width) {
        Mesh mesh;
        std::vector<Mesh::Vertex_index> vertex_indices;
        vertex_indices.reserve((path.points.size() - 2) * 2);

        const auto quads = path.points.size() - 3;
        mesh.reserve(vertex_indices.size(), 4 * quads + 1, 2 * quads);

        for (int i = 1; i < path.points.size() - 1; ++i) {
            // TODO: fix division by zero when consecutive points share same position
            Vector3 v1 = path.points[i - 1] - path.points[i];
            Vector3 v2 = path.points[i + 1] - path.points[i];
            Vector3 n1 = v1 / std::sqrt(v1.squared_length());
            Vector3 n2 = v2 / std::sqrt(v2.squared_length());
            Vector3 h = n1 + n2;
            Vector3 nh = h / std::sqrt(h.squared_length());

            const auto vertexIndexL = mesh.add_vertex(path.points[i] + nh * width);
            const auto vertexIndexR = mesh.add_vertex(path.points[i] - nh * width);

            // tests if v2 points to the left of v1 relative to the horizontal plane
            if(CGAL::scalar_product(CGAL::cross_product(v2, v1), Vector3(0,1,0)) > 0) {
                vertex_indices.push_back(vertexIndexL);
                vertex_indices.push_back(vertexIndexR);
            }else { // flip order of adding vertices to assure the vertex to the left always gets pushed first
                vertex_indices.push_back(vertexIndexR);
                vertex_indices.push_back(vertexIndexL);
            }

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
