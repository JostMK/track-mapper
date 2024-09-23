//
// Created by Jost on 09/07/2024.
//

#include "raster_reader.h"
#include "gdal_wrapper.h"

#include <array>
#include <cmath>

namespace TrackMapper::Raster {
    PointGrid readRasterData(GDALDatasetWrapper &dataset) {
        // NOTE: to conform with the coordinate system of fbx files the z axis has to be inverted

        // see: https://gdal.org/en/latest/tutorials/geotransforms_tut.html [2024-09-11]
        const GeoTransform &transform = dataset.GetGeoTransform();

        PointGrid grid;
        grid.sizeX = dataset.GetSizeX();
        grid.sizeY = dataset.GetSizeY();

        grid.origin = {transform[0], 0, -transform[3]};
        grid.projRef = dataset.GetProjectionRef();

        const std::vector<float> &values = dataset.GetData();
        grid.points.reserve(values.size());

        for (int z = 0; z < dataset.GetSizeY(); ++z) {
            for (int x = 0; x < dataset.GetSizeX(); ++x) {
                // see: https://gdal.org/tutorials/raster_api_tut.html#getting-dataset-information [2024-08-14]
                grid.points.emplace_back(transform[1] * x - z * transform[2], values[grid.GetIndex(x, z)],
                                         transform[4] * x - z * transform[5]);
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
        extends.emplace_back(transform[0] + transform[1] * 0 + 0 * transform[2],
                             transform[3] + transform[4] * 0 + 0 * transform[5]); // (0,0)
        extends.emplace_back(transform[0] + transform[1] * sizeX + 0 * transform[2],
                             transform[3] + transform[4] * sizeX + 0 * transform[5]); // (sizeX,0)
        extends.emplace_back(transform[0] + transform[1] * 0 + sizeY * transform[2],
                             transform[3] + transform[4] * 0 + sizeY * transform[5]); // (0,sizeY)
        extends.emplace_back(transform[0] + transform[1] * sizeX + sizeY * transform[2],
                             transform[3] + transform[4] * sizeX + sizeY * transform[5]); // (sizeX,sizeY)
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
        // TODO: Add bilinear interpolation
        const int x = std::floor(point.x);
        // Note: to conform with the coordinate system of fbx files the z axis is inverted so for going back into raster
        // space it needs to be inverted again
        const int y = std::floor(-point.z);

        point.y = grid.points[grid.GetIndex(x, y)].y;
    }

} // namespace TrackMapper::Raster
