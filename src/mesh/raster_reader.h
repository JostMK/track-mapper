//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_READER_H
#define RASTER_READER_H
#include <array>
#include <string>
#include <vector>

namespace TrackMapper::Mesh {
    using GeoTransform = std::array<double, 6>;

    struct RasterData {
        const std::vector<float> points;
        const int sizeX;
        const int sizeY;
        const GeoTransform transform;
    };

    RasterData readRasterData(const std::string &filepath);
}

#endif //RASTER_READER_H
