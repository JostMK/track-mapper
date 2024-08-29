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
        const GeoTransform &transform = dataset.GetGeoTransform();

        PointGrid grid;
        grid.sizeX = dataset.GetSizeX();
        grid.sizeY = dataset.GetSizeY();

        grid.origin = {transform[0], 0, transform[3]};
        grid.wkt = dataset.GetProjectionRef().exportToWkt();

        const std::vector<float> &values = dataset.GetData();
        grid.points.reserve(values.size());

        for (int z = 0; z < dataset.GetSizeY(); ++z) {
            for (int x = 0; x < dataset.GetSizeX(); ++x) {
                const int index = z * dataset.GetSizeX() + x;
                // see: https://gdal.org/tutorials/raster_api_tut.html#getting-dataset-information [2024-08-14]
                grid.points.emplace_back(transform[1] * x + z * transform[2], values[index],
                                         transform[4] * x + z * transform[5]);
            }
        }

        return grid;
    }

    bool reprojectOSMPointsIntoRaster(std::vector<OSMPoint> &points, OGRSpatialReference &dstProjRef,
                                      const Point &rasterOrigin) {
        if (dstProjRef.Validate() != OGRERR_NONE)
             return false;

        const GDALReprojectionTransformer transformer(osmPointsProjRef, dstProjRef);

        for (auto &point: points) {
            if (const bool success = transformer.Transform(&point.lat, &point.lng, nullptr); !success)
                return false;

            // align raster and path origin
            point.lat -= rasterOrigin.x;
            point.lng -= rasterOrigin.z;
        }

        return true;
    }
} // namespace TrackMapper::Raster
