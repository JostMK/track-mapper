//
// Created by Jost on 09/07/2024.
//

#include "mesh_operations.h"

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>

namespace TrackMapper::Mesh {
    Point_3 getPointForPixel(const GeoTransform &transform, double x, double y, double z);

    Mesh meshFromRasterData(const RasterData &raster_data) {
        Mesh mesh;
        std::vector<Mesh::Vertex_index> vertex_indices;
        vertex_indices.reserve(raster_data.points.size());

        const int quads = (raster_data.sizeX - 1) * (raster_data.sizeY - 1);
        mesh.reserve(
            raster_data.points.size(),
            // one triangle per quad + all the right most vertical edges + all the lowest horizontal edges
            quads * 3 + (raster_data.sizeX - 1) + (raster_data.sizeY - 1),
            quads * 2
        );

        for (int z = 0; z < raster_data.sizeY; ++z) {
            for (int x = 0; x < raster_data.sizeX; ++x) {
                const int index = z * raster_data.sizeX + x;
                const auto vertexIndex = mesh.add_vertex(
                    getPointForPixel(raster_data.transform, x, raster_data.points[index], z)
                );
                vertex_indices.push_back(vertexIndex);
            }
        }

        for (int y = 0; y < raster_data.sizeY - 1; ++y) {
            for (int x = 0; x < raster_data.sizeX - 1; ++x) {
                const int indexTL = y * raster_data.sizeX + x;
                const int indexTR = y * raster_data.sizeX + (x + 1);
                const int indexBL = (y + 1) * raster_data.sizeX + x;
                const int indexBR = (y + 1) * raster_data.sizeX + (x + 1);
                mesh.add_face(vertex_indices[indexTL], vertex_indices[indexBL], vertex_indices[indexTR]);
                mesh.add_face(vertex_indices[indexTR], vertex_indices[indexBL], vertex_indices[indexBR]);
            }
        }

        return mesh;
    }

    int reduceMesh(Mesh &mesh, const double reduction_ratio) {
        const CGAL::Surface_mesh_simplification::Edge_count_ratio_stop_predicate<Mesh> stop(reduction_ratio);
        const int edgesRemoved = CGAL::Surface_mesh_simplification::edge_collapse(mesh, stop);
        return edgesRemoved;
    }

    void writeMeshToFile(const Mesh &mesh, const std::string &filepath) {
        CGAL::IO::write_polygon_mesh(filepath, mesh, CGAL::parameters::stream_precision(17));
    }


    Point_3 getPointForPixel(const GeoTransform &transform, const double x, const double y, const double z) {
        // see: https://gdal.org/gdal.pdf#page=1002
        // FIX: temporary replace origin with 0 so values don't get to big
        // FEATURE: maybe add slight randomness to avoid grid pattern
        // origin is in the top left corner so z-axis direction is inverted
        return {
            0 + transform[1] * x - z * transform[2],
            y,
            0 + transform[4] * x - z * transform[5]
        };
    }
}
