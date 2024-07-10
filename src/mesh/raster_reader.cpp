//
// Created by Jost on 09/07/2024.
//

#include <cpl_conv.h>
#include <gdal_priv.h>

#include "raster_reader.h"

namespace TrackMapper::Mesh {
    RasterData readRasterData(const std::string &filepath) {
        CPLSetConfigOption("PROJ_LIB", "./proj");

        GDALAllRegister();

        // TODO: check for valid file
        // TODO: handle opening fails
        const auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
            GDALOpen(filepath.c_str(), GA_ReadOnly)
        ));

        std::array<double, 6> transform{};
        pDataset->GetGeoTransform(transform.data());

        // FEATURE: supports only one raster band per raster file
        const auto band = pDataset->GetRasterBand(1);

        const int nXSize = band->GetXSize();
        const int nYSize = band->GetYSize();

        std::vector<float> values(nXSize * nYSize);

        // TODO: make type of data dynamic based on file
        if (const auto err = band->RasterIO(GF_Read, 0, 0, nXSize, nYSize, values.data(), nXSize, nYSize, GDT_Float32,
                                            0, 0);
            err != CE_None) {
            // TODO: handle error
        }

        return {values, nXSize, nYSize, transform};
    }
}


