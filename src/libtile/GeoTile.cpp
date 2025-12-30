//
// Created by Jost on 18/09/2025.
//

#include "GeoTile.h"

#include <gdal_alg.h>
#include <gdal_priv.h>

namespace LibTile {
    void normalize_tile_data(GeoTile &tile) {
        if (tile.geo_transform.x_scale < 0) {
            tile.geo_transform.x_origin += tile.size_x * tile.geo_transform.x_scale;
            tile.geo_transform.x_scale *= -1;

            // reverse data in x direction
            for (int y = 0; y < tile.size_y; y++) {
                int start = y * tile.size_x;
                int end = (y + 1) * tile.size_x - 1;
                for (int x = 0; x < tile.size_x / 2; x++) {
                    std::swap(tile.height[start], tile.height[end]);
                    start += 1;
                    end -= 1;
                }
            }
        }

        if (tile.geo_transform.y_scale > 0) {
            tile.geo_transform.y_origin += tile.size_y * tile.geo_transform.y_scale;
            tile.geo_transform.y_scale *= -1;

            // reverse data in y direction
            for (int x = 0; x < tile.size_x; x++) {
                int start = x;
                int end = static_cast<int>(tile.height.size()) - tile.size_x + x;
                for (int y = 0; y < tile.size_y / 2; y++) {
                    std::swap(tile.height[start], tile.height[end]);
                    start += tile.size_x;
                    end -= tile.size_x;
                }
            }
        }
    }

    std::expected<GeoTile, Error> load_from_file(const std::string &filepath, const bool normalize_direction) {
        static bool gdal_configured = false;
        if (!gdal_configured) {
            CPLSetConfigOption("PROJ_LIB", "./proj");
            GDALAllRegister();

            gdal_configured = true;
        }

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

        if (const auto error = band->RasterIO(GF_Read, 0, 0, tile.size_x, tile.size_y, tile.height.data(), tile.size_x,
                                              tile.size_y, GDT_Float32, 0, 0);
            error != CPLE_None) {
            pDataset->Close();
            return std::unexpected(Error::FAILED_TO_READ_DATA);
        }

        pDataset->Close();

        if (normalize_direction) {
            normalize_tile_data(tile);
        }

        return tile;
    }
} // namespace LibTile
