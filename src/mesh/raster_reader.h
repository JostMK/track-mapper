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

        Point operator-() const { return Point{-x, -y, -z}; }
        Point operator+(const Point &other) const { return Point{x + other.x, y + other.y, z + other.z}; }
        Point operator-(const Point &other) const { return Point{x - other.x, y - other.y, z - other.z}; }
        Point operator*(const double factor) const { return Point{x * factor, y * factor, z * factor}; }

        [[nodiscard]] double SqLength() const { return x * x + y * y + z * z; }
        [[nodiscard]] double Length() const { return sqrt(SqLength()); }
    };
    inline Point operator*(const double factor, const Point &point) {
        return Point{point.x * factor, point.y * factor, point.z * factor};
    }

    struct PointGrid {
        std::vector<Point> points;
        int sizeX, sizeY;
        Point origin;
        std::string wkt; // projection information

        [[nodiscard]] int GetIndex(const int x, const int y) const { return y * sizeX + x; }
    };

    PointGrid readRasterData(GDALDatasetWrapper &dataset);

    bool reprojectOSMPoints(std::vector<OSMPoint> &points, OGRSpatialReference &dstProjRef);

    void interpolateHeightInGrid(const PointGrid &grid, Point &point);

} // namespace TrackMapper::Raster

#endif // RASTER_READER_H
