//
// Created by Jost on 22/07/2024.
// Modified from https://doc.cgal.org/latest/Surface_mesh_simplification/index.html example 5.9
//

#include <iostream>

#include "../../src/mesh/gdal_wrapper.h"

int main() {
    std::cout << "Input geo raster image path: " << std::endl;
    std::string inFilename;
    std::cin >> inFilename;

    const TrackMapper::Raster::GDALDatasetWrapper dataset(inFilename);

    if(!dataset.IsValid()) {
        std::cout << "Provided path could not be opened: " << inFilename << std::endl;
        return 1;
    }

    // see: https://gdal.org/en/latest/tutorials/geotransforms_tut.html [2024-09-27]
    const TrackMapper::Raster::GeoTransform &transform = dataset.GetGeoTransform();

    const double originX = transform[0];
    const double originY = transform[3];

    const double pixelStepX = transform[1];
    const double pixelStepY = transform[5];

    // in case of y axis aligned with north-south axis these should be zero
    const double rotationX = transform[2];
    const double rotationY = transform[4];

    std::cout << std::format("Origin: {} : {}", originX, originY) << std::endl;
    std::cout << std::format("Pixel Step: {} : {}", pixelStepX, pixelStepY) << std::endl;
    std::cout << std::format("Rotation: {} : {}", rotationX, rotationY) << std::endl;
}
