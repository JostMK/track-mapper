//
// Created by Jost on 12/08/2024.
//

#ifndef GDAL_WRAPPER_H
#define GDAL_WRAPPER_H

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <ogr_spatialref.h>

namespace TrackMapper::Raster {

    using GeoTransform = std::array<double, 6>;

    // TODO: re-add wrapper around OGRSpatialReference (problem: inconsistent behaviour when using for reprojection)

    /// A wrapper around the GDALDatasetUniquePtr class
    class GDALDatasetWrapper {
    public:
        /**
         * @param filepath Path to a geo raster dataset file readable by GDAL (e.g. .tif (geo tiff), .xyz, ..)
         */
        explicit GDALDatasetWrapper(const std::string &filepath);
        ~GDALDatasetWrapper();

        /**
         * @return GeoTransform containing tranformation information of the dataset
         * @see [Introduction to Geotransforms](https://gdal.org/tutorials/geotransforms_tut.html#geotransform-tutorial)
         */
        [[nodiscard]] const GeoTransform &GetGeoTransform() const;

        [[nodiscard]] int GetSizeX() const;
        [[nodiscard]] int GetSizeY() const;

        /**
         * @return vector containing the hight data for each pixel in row major order
         * @note The data is in row major (x-axis) order
         * @note At the moment only reading data from the first raster band is supported
         * @note At the moment only data of type float (GDT_Float32) is supported
         */
        const std::vector<float> &GetData();

        [[nodiscard]] const OGRSpatialReference &GetProjectionRef() const;

    private:
        // opaque pointer to avoid including gdal headers
        struct impl;
        std::unique_ptr<impl> pImpl;

        GeoTransform mTransform;
        OGRSpatialReference mProjRef;
        std::vector<float> mData;
    };

    /// A wrapper around GDALCreateReprojectionTransformerEx class and GDALReprojectionTransform function
    class GDALReprojectionTransformer {
    public:
        GDALReprojectionTransformer(OGRSpatialReference &srcProjRef, OGRSpatialReference &dstProjRef);
        ~GDALReprojectionTransformer();

        [[nodiscard]] bool Transform(double *x, double *y, double *z) const;

        [[nodiscard]] bool IsValid() const;

    private:
        // opaque pointer to avoid including gdal headers
        struct impl;
        std::unique_ptr<impl> pImpl;
    };
} // namespace TrackMapper::Raster

#endif // GDAL_WRAPPER_H
