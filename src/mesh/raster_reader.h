//
// Created by Jost on 09/07/2024.
//

#ifndef RASTER_READER_H
#define RASTER_READER_H

#include <string>
#include <vector>

#include "gdal_wrapper.h"

namespace TrackMapper::Raster {

    inline ProjectionWrapper osmPointsProjRef(
            R"(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])");

    struct OSMPoint {
        double lat, lng;
    };

    struct Point {
        double x, y, z;

        Point() = default;
        Point(double x, double y, double z);

        Point operator-() const { return Point{-x, -y, -z}; }
        Point operator+(const Point &other) const { return Point{x + other.x, y + other.y, z + other.z}; }
        Point operator-(const Point &other) const { return Point{x - other.x, y - other.y, z - other.z}; }
        Point operator*(const double factor) const { return Point{x * factor, y * factor, z * factor}; }
        void operator+=(const Point &other) {
            x += other.x;
            y += other.y;
            z += other.z;
        }
        void operator-=(const Point &other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
        }

        [[nodiscard]] double SqLength() const { return x * x + y * y + z * z; }
        [[nodiscard]] double Length() const { return sqrt(SqLength()); }
    };
    inline Point operator*(const double factor, const Point &point) {
        return Point{point.x * factor, point.y * factor, point.z * factor};
    }

    struct PointGrid {
        // only supports non roateted/skewed rasters
        std::vector<Point> points;
        int sizeX, sizeY;
        double pixelSizeX, pixelSizeY;
        Point origin;
        ProjectionWrapper projRef;
        GeoTransform transform;

        [[nodiscard]] int GetIndex(const int x, const int y) const { return y * sizeX + x; }
    };

    PointGrid readRasterData(GDALDatasetWrapper &dataset);

    std::vector<OSMPoint> getDatasetExtends(const GDALDatasetWrapper &dataset);

    bool reprojectOSMPoints(std::vector<OSMPoint> &points, const ProjectionWrapper &dstProjRef);

    bool reprojectPoints(std::vector<OSMPoint> &points, const ProjectionWrapper &srcProjRef,
                         const ProjectionWrapper &dstProjRef);

    void SetHeightFromGrid(const PointGrid &grid, Point &point);

    double GetHeightForPointInGrid(const PointGrid &grid, const Point &point);

    Point getRasterPoint(const GeoTransform &transform, int pixelX, int pixelY, bool raw = false);

} // namespace TrackMapper::Raster

#endif // RASTER_READER_H
