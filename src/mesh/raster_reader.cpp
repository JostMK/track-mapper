//
// Created by Jost on 09/07/2024.
//

#include <cpl_conv.h>
#include <gdal_alg.h>
#include <gdal_priv.h>

#include "raster_reader.h"

#include <array>
#include <iostream>

namespace TrackMapper::Raster {

    using GeoTransform = std::array<double, 6>;

    OGRSpatialReference osmPointsSpatRef(
            R"(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])");

    Point getPointForPixel(const GeoTransform &transform, double x, double y, double z);

    PointGrid readRasterData(const std::string &rasterFilePath) {
        CPLSetConfigOption("PROJ_LIB", "./proj");
        GDALAllRegister();

        // TODO: check for valid file
        // TODO: handle opening fails
        const auto pDataset =
                GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(rasterFilePath.c_str(), GA_ReadOnly)));

        GeoTransform transform{};
        pDataset->GetGeoTransform(transform.data());

        // FEATURE: supports more than one raster band per raster file
        // RasterBand numbering starts with 1 see: https://gdal.org/gdal.pdf#page=1126
        const auto band = pDataset->GetRasterBand(1);

        const int nXSize = band->GetXSize();
        const int nYSize = band->GetYSize();

        std::vector<float> values(nXSize * nYSize);

        // TODO: make type of data dynamic based on file
        if (const auto err =
                    band->RasterIO(GF_Read, 0, 0, nXSize, nYSize, values.data(), nXSize, nYSize, GDT_Float32, 0, 0);
            err != CE_None) {
            // TODO: handle error
        }

        PointGrid grid;
        grid.sizeX = nXSize;
        grid.sizeY = nYSize;

        grid.origin = {transform[0], 0, transform[3]};

        const OGRSpatialReference dbSpatialRef(pDataset->GetProjectionRef());
        grid.wkt = dbSpatialRef.exportToWkt();

        for (int z = 0; z < nYSize; ++z) {
            for (int x = 0; x < nXSize; ++x) {
                const int index = z * nXSize + x;
                auto point = getPointForPixel(transform, x, values[index], z);
                grid.points.push_back(point);
            }
        }

        return grid;
    }

    Point getPointForPixel(const GeoTransform &transform, const double x, const double y, const double z) {
        // see: https://gdal.org/gdal.pdf#page=1002
        // FEATURE: maybe add slight randomness to avoid grid pattern
        // origin is in the top left corner so z-axis direction is inverted
        return {
                transform[1] * x + z * transform[2], //
                y, //
                transform[4] * x + z * transform[5] //
        };
    }


    bool reprojectPointsIntoRaster(const std::string &rasterFilePath, std::vector<Point> &points) {
        CPLSetConfigOption("PROJ_LIB", "./proj");
        GDALAllRegister();

        // TODO: check for valid file
        // TODO: handle opening fails
        const auto pDataset =
                GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(rasterFilePath.c_str(), GA_ReadOnly)));

        GeoTransform transform{};
        pDataset->GetGeoTransform(transform.data());

        OGRSpatialReference dstSpatialRef(pDataset->GetProjectionRef());
        if (dstSpatialRef.Validate() != OGRERR_NONE)
            return false;

        const auto pTransform =
                GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&osmPointsSpatRef),
                                                    OGRSpatialReference::ToHandle(&dstSpatialRef), nullptr);
        if (pTransform == nullptr)
            return false;

        for (auto &point: points) {
            bool success = GDALReprojectionTransform(pTransform, 0, 1, &point.x, &point.y, nullptr, nullptr);

            // align raster and path origin
            point.x -= transform[0];
            point.y -= transform[3];
            if (!success)
                return false;
        }

        return true;
    }
} // namespace TrackMapper::Raster
