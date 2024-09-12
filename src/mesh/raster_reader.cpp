//
// Created by Jost on 09/07/2024.
//

#include "raster_reader.h"
#include "gdal_wrapper.h"

#include <array>
#include <iostream>

namespace TrackMapper::Raster {

    OGRSpatialReference osmPointsProjRef(
            R"(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])");

    PointGrid readRasterData(GDALDatasetWrapper &dataset) {
        // NOTE: to conform with the coordinate system of fbx files the z axis has to be inverted

        // see: https://gdal.org/en/latest/tutorials/geotransforms_tut.html [2024-09-11]
        const GeoTransform &transform = dataset.GetGeoTransform();

        PointGrid grid;
        grid.sizeX = dataset.GetSizeX();
        grid.sizeY = dataset.GetSizeY();

        grid.origin = {transform[0], 0, -transform[3]};
        grid.wkt = dataset.GetProjectionRef().exportToWkt();

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

    bool reprojectOSMPoints(std::vector<OSMPoint> &points, OGRSpatialReference &dstProjRef) {
        if (dstProjRef.Validate() != OGRERR_NONE)
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

    void interpolateHeightInGrid(const PointGrid &grid, Point &point) {
        // TODO: Add bilinear interpolation
        const int x = std::floor(point.x);
        // Note: to conform with the coordinate system of fbx files the z axis is inverted so for going back into raster
        // space it needs to be inverted again
        const int z = std::floor(-point.z);

        point.y = grid.points[grid.GetIndex(x, z)].y;
    }

} // namespace TrackMapper::Raster
