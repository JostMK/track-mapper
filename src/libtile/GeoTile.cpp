//
// Created by Jost on 18/09/2025.
//

#include "GeoTile.h"

#include <gdal_alg.h>
#include <gdal_priv.h>

std::expected<LibTile::GeoTile, LibTile::Error> LibTile::load_from_file(const std::string &filepath) {
    // TODO: maybe find better way to initialise GDAL
    CPLSetConfigOption("PROJ_LIB", "./proj");
    GDALAllRegister();

    const auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(filepath.c_str(), GA_ReadOnly)));

    if (!pDataset) {
        return std::unexpected(Error::FAILED_TO_OPEN_DATASET);
    }

    GeoTile tile;

    tile.proj_wkt = pDataset->GetProjectionRef();

    if (const auto error = pDataset->GetGeoTransform(tile.geo_transform.values.data()); error != CPLE_None) {
        pDataset->Close();
        return std::unexpected(Error::FAILED_TO_GET_GEO_TRANSFORM);
    }

    // RasterBand numbering starts with 1
    // see: https://gdal.org/tutorials/raster_api_tut.html#fetching-a-raster-band [2024-08-14]
    const auto band = pDataset->GetRasterBand(1);

    tile.size_x = band->GetXSize();
    tile.size_y = band->GetYSize();
    tile.height.resize(tile.size_x * tile.size_y);

    // TODO: make type of data dynamic based on file info using band->GetRasterDataType()
    if (const auto error = band->RasterIO(GF_Read, 0, 0, tile.size_x, tile.size_y, tile.height.data(), tile.size_x,
                                          tile.size_y, GDT_Float32, 0, 0);
        error != CPLE_None) {
        pDataset->Close();
        return std::unexpected(Error::FAILED_TO_READ_DATA);
    }

    pDataset->Close();
    return tile;
}
