//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_READER_H
#define RASTER_READER_H

#include <string>
#include <vector>

#include "gdal_wrapper.h"

namespace TrackMapper::Raster {

    struct OSMPoint {
        double lat, lng;
    };

    struct Point {
        double x, y, z;
    };

    struct PointGrid {
        std::vector<Point> points;
        int sizeX, sizeY;
        Point origin;
        std::string wkt; // projection information
    };

    PointGrid readRasterData(GDALDatasetWrapper &dataset);

    bool reprojectOSMPointsIntoRaster(std::vector<OSMPoint> &points, OGRSpatialReference &dstProjRef,
                                      const Point &rasterOrigin);

} // namespace TrackMapper::Raster

#endif // RASTER_READER_H
