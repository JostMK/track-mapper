//
// Created by Jost on 12/08/2024.
//

#include "gdal_wrapper.h"

#include <gdal_alg.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <utility>

namespace TrackMapper::Raster {

    // ----- ProjectionWrapper -----

    ProjectionWrapper::ProjectionWrapper() = default;
    ProjectionWrapper::ProjectionWrapper(std::string wkt) : mWKT(std::move(wkt)) {
        const OGRSpatialReference spatRef(Get().c_str());
        mValid = spatRef.Validate() == OGRERR_NONE;
    }

    std::string ProjectionWrapper::Get() const {
        const auto copy = mWKT;
        return copy;
    }

    bool ProjectionWrapper::IsValid() const { return mValid; }


    // ----- GDALDatasetWrapper -----

    struct GDALDatasetWrapper::impl {
        GDALDatasetUniquePtr pDataset;

        explicit GDALDatasetWrapper::impl(GDALDatasetUniquePtr pDataset) : pDataset{std::move(pDataset)} {}
    };

    GDALDatasetWrapper::GDALDatasetWrapper(const std::string &filepath) : mTransform(), mProjRef{""} {
        CPLSetConfigOption("PROJ_LIB", "./proj");
        GDALAllRegister();

        auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(filepath.c_str(), GA_ReadOnly)));

        if (!pDataset) {
            invalid = true;
            return;
        }
        invalid = false;

        pDataset->GetGeoTransform(mTransform.data());
        mProjRef = ProjectionWrapper(pDataset->GetProjectionRef());

        // moving ownership of the dataset to impl struct
        pImpl = std::make_unique<impl>(std::move(pDataset));
        pDataset = nullptr;
    }

    GDALDatasetWrapper::~GDALDatasetWrapper() {
        // TODO: handle possible errors
        if (!invalid)
            pImpl->pDataset->Close();
    }
    bool GDALDatasetWrapper::IsValid() const { return !invalid; }

    const GeoTransform &GDALDatasetWrapper::GetGeoTransform() const { return mTransform; }

    int GDALDatasetWrapper::GetSizeX() const {
        if (invalid)
            return 0;

        return pImpl->pDataset->GetRasterBand(1)->GetXSize();
    }

    int GDALDatasetWrapper::GetSizeY() const {
        if (invalid)
            return 0;

        return pImpl->pDataset->GetRasterBand(1)->GetYSize();
    }

    const std::vector<float> &GDALDatasetWrapper::GetData() {
        if (invalid)
            return mData; // will be empty

        if (!mData.empty())
            return mData;

        // RasterBand numbering starts with 1
        // see: https://gdal.org/tutorials/raster_api_tut.html#fetching-a-raster-band [2024-08-14]
        const auto band = pImpl->pDataset->GetRasterBand(1);
        const int nXSize = band->GetXSize();
        const int nYSize = band->GetYSize();

        mData.resize(nXSize * nYSize);

        // TODO: make type of data dynamic based on file info
        band->RasterIO(GF_Read, 0, 0, nXSize, nYSize, mData.data(), nXSize, nYSize, GDT_Float32, 0, 0);
        // TODO: handle error

        return mData;
    }

    const ProjectionWrapper &GDALDatasetWrapper::GetProjectionRef() const { return mProjRef; }


    // ----- GDALReprojectionTransformer -----

    struct GDALReprojectionTransformer::impl {
        void *transformer; // GDALCreateReprojectionTransformer

        explicit GDALDatasetWrapper::impl(void *transformer) : transformer(transformer) {}
    };

    GDALReprojectionTransformer::GDALReprojectionTransformer(const ProjectionWrapper &srcProjRef,
                                                             const ProjectionWrapper &dstProjRef) {

        OGRSpatialReference srcProj(srcProjRef.Get().c_str());
        OGRSpatialReference dstProj(dstProjRef.Get().c_str());

        auto transformer = GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&srcProj),
                                                               OGRSpatialReference::ToHandle(&dstProj), nullptr);
        pImpl = std::make_unique<impl>(transformer);
    }

    GDALReprojectionTransformer::~GDALReprojectionTransformer() {
        GDALDestroyReprojectionTransformer(pImpl->transformer);
    }

    bool GDALReprojectionTransformer::Transform(double *x, double *y, double *z) const {
        if (pImpl->transformer == nullptr)
            return false;

        return GDALReprojectionTransform(pImpl->transformer, 0, 1, x, y, z, nullptr);
    }

    bool GDALReprojectionTransformer::IsValid() const { return pImpl->transformer != nullptr; }

} // namespace TrackMapper::Raster
