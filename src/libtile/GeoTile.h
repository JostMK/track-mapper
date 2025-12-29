//
// Created by Jost on 16/09/2025.
//

#ifndef GEOTILE_H
#define GEOTILE_H

#include <array>
#include <expected>
#include <string>
#include <vector>

namespace LibTile {

    /// GDAL GeoTransform
    /// <see cref="https://gdal.org/en/stable/tutorials/geotransforms_tut.html"/>
    union GeoTransform {
        struct {
            double x_origin;
            double x_scale;
            double x_skew;
            double y_origin;
            double y_skew;
            double y_scale;
        };
        std::array<double, 6> values{};

        void transform(double &x, double &y) const {
            x = x_origin + x * x_scale + y * x_skew;
            y = y_origin + y * y_scale + x * y_skew;
        }

        void transform_relative(double &x, double &y) const {
            x = x * x_scale + y * x_skew;
            y = y * y_scale + x * y_skew;
        }

        void get_origin(double &x, double &y) const {
            x = x_origin;
            y = y_origin;
        }
    };

    struct GeoTile {
        /// OpenGIS Spatial Reference System in form of OGC WKT
        /// <see cref="https://gdal.org/en/stable/tutorials/osr_api_tut.html"/>
        std::string proj_wkt;

        GeoTransform geo_transform;

        int size_x;
        int size_y;

        std::vector<float> height;
    };

    enum Error {
       FAILED_TO_OPEN_DATASET,
       FAILED_TO_GET_GEO_TRANSFORM,
       FAILED_TO_READ_DATA,
    };

    std::expected<GeoTile, Error> load_from_file(const std::string& filepath);
} // namespace LibTile

#endif // GEOTILE_H
