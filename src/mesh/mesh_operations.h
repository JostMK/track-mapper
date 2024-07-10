//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_TO_MESH_H
#define RASTER_TO_MESH_H

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Point_3.h>
#include <CGAL/Surface_mesh/Surface_mesh.h>

#include "raster_reader.h"

namespace TrackMapper::Mesh {
    using Kernel = CGAL::Simple_cartesian<double>;
    using Point_3 = Kernel::Point_3;
    using Mesh = CGAL::Surface_mesh<Point_3>;

    Mesh meshFromRasterData(const RasterData &raster_data);

    int reduceMesh(Mesh &mesh, double reduction_ratio);

    void writeMeshToFile(const Mesh &mesh, const std::string &filepath);

}

#endif //RASTER_TO_MESH_H
