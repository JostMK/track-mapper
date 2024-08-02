//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_READER_H
#define RASTER_READER_H

#include <string>
#include <vector>

namespace TrackMapper::Raster {

    struct Point {
        double x, y, z;
    };

    struct PointGrid {
        std::vector<Point> points;
        int sizeX, sizeY;
        Point origin;
        std::string wkt; // projection information
    };

    PointGrid readRasterData(const std::string &rasterFilePath);

    bool reprojectPointsIntoRaster(const std::string &rasterFilePath, std::vector<Point> &points);
} // namespace TrackMapper::Raster

#endif // RASTER_READER_H
