//
// Created by Jost on 09/07/2024.
//

#include <cpl_conv.h>
#include <gdal_alg.h>
#include <gdal_priv.h>

#include "raster_reader.h"

#include <iostream>

namespace TrackMapper::Raster {

    using GeoTransform = std::array<double, 6>;

    Point getPointForPixel(const GeoTransform &transform, double x, double y, double z);

    PointGrid readRasterData(const std::string &filepath) {
        CPLSetConfigOption("PROJ_LIB", "./proj");

        GDALAllRegister();

        // TODO: check for valid file
        // TODO: handle opening fails
        const auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(filepath.c_str(), GA_ReadOnly)));

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

        // reproject points into mercator projection
        const OGRSpatialReference wktRef(pDataset->GetProjectionRef());
        const auto pTransform = GDALCreateReprojectionTransformer(wktRef.exportToWkt().c_str(), "merc");

        if(pTransform == nullptr) {
            std::cout << "Error creating transformer" << std::endl;
        }

        for (int z = 0; z < nYSize; ++z) {
            for (int x = 0; x < nXSize; ++x) {
                const int index = z * nXSize + x;
                auto point = getPointForPixel(transform, x, values[index], z);
                int _s = 0;
                bool success = GDALReprojectionTransform(pTransform, 0, 1, &point.x, &point.y, &point.z, &_s);

                if(!success)
                    std::cout << "Error reprojecting" << std::endl;

                grid.points.push_back(point);
            }
        }

        GDALDestroyReprojectionTransformer(pTransform);

        return grid;
    }

    Point getPointForPixel(const GeoTransform &transform, const double x, const double y, const double z) {
        // see: https://gdal.org/gdal.pdf#page=1002
        // FEATURE: maybe add slight randomness to avoid grid pattern
        // origin is in the top left corner so z-axis direction is inverted
        return {
                transform[0] + transform[1] * x - z * transform[2], //
                y, //
                transform[3] + transform[4] * x - z * transform[5] //
        };
    }
} // namespace TrackMapper::Mesh::Raster
