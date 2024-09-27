//
// Created by Jost on 09/07/2024.
//

#include "raster_reader.h"
#include "gdal_wrapper.h"

#include <array>
#include <cmath>

namespace TrackMapper::Raster {
    PointGrid readRasterData(GDALDatasetWrapper &dataset) {
        const GeoTransform &transform = dataset.GetGeoTransform();
        // Todo: handle rotated or skewed raster images (transform[2] and transform[4] are non zero)

        PointGrid grid;
        grid.sizeX = dataset.GetSizeX();
        grid.sizeY = dataset.GetSizeY();
        grid.pixelSizeX = transform[1];
        grid.pixelSizeY = transform[5];

        // ensure that origin is always at the top left of the raster
        const bool flippedZOrigin = grid.pixelSizeY > 0;

        if (flippedZOrigin) {
            grid.origin = getRasterPoint(transform, 0, grid.sizeY - 1);
            grid.pixelSizeY *= -1;
        } else {
            grid.origin = getRasterPoint(transform, 0, 0);
        }

        grid.projRef = dataset.GetProjectionRef();
        grid.transform = transform;

        const std::vector<float> &values = dataset.GetData();
        grid.points.reserve(values.size());

        if (flippedZOrigin) {
            for (int z = 0; z < dataset.GetSizeY(); ++z) {
                for (int x = 0; x < dataset.GetSizeX(); ++x) {
                    const auto coordZ = dataset.GetSizeY() - 1 - z;
                    auto p = getRasterPoint(transform, x, coordZ) - grid.origin;
                    const auto height = values[grid.GetIndex(x, coordZ)];
                    p.y = height;
                    grid.points.push_back(p);
                }
            }
        } else {
            for (int z = 0; z < dataset.GetSizeY(); ++z) {
                for (int x = 0; x < dataset.GetSizeX(); ++x) {
                    auto p = getRasterPoint(transform, x, z) - grid.origin;
                    const auto height = values[grid.GetIndex(x, z)];
                    p.y = height;
                    grid.points.push_back(p);
                }
            }
        }

        return grid;
    }

    std::vector<OSMPoint> getDatasetExtends(const GDALDatasetWrapper &dataset) {
        // NOTE: this function does NOT conform with the inverted z coordinate system of fbx scenes

        // see: https://gdal.org/en/latest/tutorials/geotransforms_tut.html [2024-09-11]
        const GeoTransform &transform = dataset.GetGeoTransform();
        const int sizeX = dataset.GetSizeX();
        const int sizeY = dataset.GetSizeY();

        std::vector<OSMPoint> extends;
        auto p = getRasterPoint(transform, 0, 0, true);
        extends.emplace_back(p.x, p.z); // (0, 0)

        p = getRasterPoint(transform, sizeX, 0, true);
        extends.emplace_back(p.x, p.z); // (sizeX, 0)

        p = getRasterPoint(transform, 0, sizeY, true);
        extends.emplace_back(p.x, p.z); // (0, sizeY)

        p = getRasterPoint(transform, sizeX, sizeY, true);
        extends.emplace_back(p.x, p.z); // (sizeX, sizeY)

        return extends;
    }

    bool reprojectOSMPoints(std::vector<OSMPoint> &points, const ProjectionWrapper &dstProjRef) {
        if (!dstProjRef.IsValid())
            return false;

        const GDALReprojectionTransformer transformer(osmPointsProjRef, dstProjRef);

        for (auto &[lat, lng]: points) {
            if (const bool success = transformer.Transform(&lat, &lng, nullptr); !success)
                return false;

            // Note: to conform with the coordinate system of fbx files the z axis has to be inverted
            lng *= -1;
        }

        return true;
    }
    bool reprojectPoints(std::vector<OSMPoint> &points, const ProjectionWrapper &srcProjRef,
                         const ProjectionWrapper &dstProjRef) {
        if (!srcProjRef.IsValid())
            return false;

        if (!dstProjRef.IsValid())
            return false;

        const GDALReprojectionTransformer transformer(srcProjRef, dstProjRef);

        for (auto &[lat, lng]: points) {
            if (const bool success = transformer.Transform(&lat, &lng, nullptr); !success)
                return false;
        }

        return true;
    }

    void interpolateHeightInGrid(const PointGrid &grid, Point &point) {
        // TODO: this can only handle north aligned transforms - not transforms with rotation or shearing
        // TODO: Add bilinear interpolation
        const int xIndex = std::floor(point.x / grid.transform[1]);
        // Note: to conform with the coordinate system of fbx files the z axis is inverted so for going back into raster
        // space it needs to be inverted again
        const int yIndex = std::floor(-point.z / grid.transform[5]);

        point.y = grid.points[grid.GetIndex(xIndex, yIndex)].y;
    }

    Point getRasterPoint(const GeoTransform &transform, const int pixelX, const int pixelY, const bool raw) {
        // NOTE: to conform with the coordinate system of fbx files the z axis has to be inverted
        const double zSign = raw ? 1 : -1;

        // see: https://gdal.org/en/latest/tutorials/geotransforms_tut.html [2024-09-11]
        // see: https://gdal.org/tutorials/raster_api_tut.html#getting-dataset-information [2024-08-14
        return {
                transform[0] + transform[1] * pixelX + zSign * pixelY * transform[2], //
                0, //
                zSign * transform[3] + transform[4] * pixelX + zSign * pixelY * transform[5] //
        };
    }

} // namespace TrackMapper::Raster
