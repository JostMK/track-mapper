//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_READER_H
#define RASTER_READER_H
#include <array>
#include <string>
#include <vector>

namespace TrackMapper::Raster {

    struct Point {
        double x,y,z;
    };

    struct PointGrid {
        std::vector<Point> points;
        int sizeX, sizeY;
    };

    PointGrid readRasterData(const std::string &filepath);
}

#endif //RASTER_READER_H
