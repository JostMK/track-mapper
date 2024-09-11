//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_TO_MESH_H
#define RASTER_TO_MESH_H

#include <CGAL/Point_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh/Surface_mesh.h>

#include "raster_reader.h"

namespace TrackMapper::Mesh {
    using CGALKernel = CGAL::Simple_cartesian<double>;
    using CGALPoint3 = CGALKernel::Point_3;
    using CGALMesh = CGAL::Surface_mesh<CGALPoint3>;

    struct Path {
        std::vector<CGALPoint3> points;
    };

    CGALMesh meshFromRasterData(const Raster::PointGrid &point_grid);

    CGALMesh meshFromPath(const Path &path, double width, int subdivisions);

    int reduceMesh(CGALMesh &mesh, double reduction_ratio);

    void writeMeshToFile(const CGALMesh &mesh, const std::string &filepath);

} // namespace TrackMapper::Mesh

#endif // RASTER_TO_MESH_H
