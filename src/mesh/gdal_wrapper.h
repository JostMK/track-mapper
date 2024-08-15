//
// Created by Jost on 12/08/2024.
//

#ifndef GDAL_WRAPPER_H
#define GDAL_WRAPPER_H

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace TrackMapper::Raster {

    using GeoTransform = std::array<double, 6>;

    /// A wrapper around the OGRSpatialReference class
    class GDALProjectionReferenceWrapper {
    public:
        /**
         * @param wktString A string in the OGC WKT format specifying a projection
         */
        explicit GDALProjectionReferenceWrapper(const std::string &wktString);
        ~GDALProjectionReferenceWrapper(); // needed for pImpl to compile!

        GDALProjectionReferenceWrapper(const GDALProjectionReferenceWrapper &other); // copy constructor
        GDALProjectionReferenceWrapper(GDALProjectionReferenceWrapper &&other) noexcept; // move constructor
        GDALProjectionReferenceWrapper &operator=(const GDALProjectionReferenceWrapper &other); // copy assignment
        GDALProjectionReferenceWrapper &operator=(GDALProjectionReferenceWrapper &&other) noexcept; // move assignment

        [[nodiscard]] std::string GetWkt() const;

        [[nodiscard]] bool IsValid() const;

    private:
        // opaque pointer to avoid including gdal headers
        struct impl;
        std::unique_ptr<impl> pImpl;
    };

    /// A wrapper around the GDALDatasetUniquePtr class
    class GDALDatasetWrapper {
    public:
        explicit GDALDatasetWrapper(const std::string &filepath);
        ~GDALDatasetWrapper();

        [[nodiscard]] const GeoTransform &GetGeoTransform() const;

        [[nodiscard]] int GetSizeX() const;
        [[nodiscard]] int GetSizeY() const;

        /// Returns a vector containing the data of the first raster band.
        /// Note: The data is in row major (x-axis) order
        /// Note: At the moment only reading data from the first raster band is supported
        /// Note: At the moment only data of type float (GDT_Float32) is supported
        const std::vector<float> &GetData();

        [[nodiscard]] const GDALProjectionReferenceWrapper &GetProjectionRef() const;

    private:
        // opaque pointer to avoid including gdal headers
        struct impl;
        std::unique_ptr<impl> pImpl;

        GeoTransform mTransform;
        GDALProjectionReferenceWrapper mProjRef;
        std::vector<float> mData;
    };

    /// A wrapper around GDALCreateReprojectionTransformerEx class and GDALReprojectionTransform function
    class GDALReprojectionTransformer {
    public:
        GDALReprojectionTransformer(const GDALProjectionReferenceWrapper &srcProjRef,
                                    const GDALProjectionReferenceWrapper &dstProjRef);
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
