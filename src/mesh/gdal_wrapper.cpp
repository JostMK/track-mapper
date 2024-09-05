//
// Created by Jost on 12/08/2024.
//

#include "gdal_wrapper.h"

#include <gdal_alg.h>
#include <gdal_priv.h>

namespace TrackMapper::Raster {

    // ----- GDALDatasetWrapper -----

    struct GDALDatasetWrapper::impl {
        GDALDatasetUniquePtr pDataset;

        explicit GDALDatasetWrapper::impl(GDALDatasetUniquePtr pDataset) : pDataset{std::move(pDataset)} {}
    };

    GDALDatasetWrapper::GDALDatasetWrapper(const std::string &filepath) : mTransform(), mProjRef{""} {
        CPLSetConfigOption("PROJ_LIB", "./proj");
        GDALAllRegister();

        // TODO: handle error with invalid filepath
        auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(filepath.c_str(), GA_ReadOnly)));

        pDataset->GetGeoTransform(mTransform.data());
        mProjRef = OGRSpatialReference(pDataset->GetProjectionRef());

        // moving ownership of the dataset to impl struct
        pImpl = std::make_unique<impl>(std::move(pDataset));
        pDataset = nullptr;
    }

    GDALDatasetWrapper::~GDALDatasetWrapper() {
        // TODO: handle possible errors
        pImpl->pDataset->Close();
    }

    const GeoTransform &GDALDatasetWrapper::GetGeoTransform() const { return mTransform; }

    int GDALDatasetWrapper::GetSizeX() const { return pImpl->pDataset->GetRasterBand(1)->GetXSize(); }

    int GDALDatasetWrapper::GetSizeY() const { return pImpl->pDataset->GetRasterBand(1)->GetYSize(); }

    const std::vector<float> &GDALDatasetWrapper::GetData() {
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

    const OGRSpatialReference &GDALDatasetWrapper::GetProjectionRef() const { return mProjRef; }


    // ----- GDALReprojectionTransformer -----

    struct GDALReprojectionTransformer::impl {
        void *transformer; // GDALCreateReprojectionTransformer

        explicit GDALDatasetWrapper::impl(void *transformer) : transformer(transformer) {}
    };

    GDALReprojectionTransformer::GDALReprojectionTransformer(OGRSpatialReference &srcProjRef,
                                                             OGRSpatialReference &dstProjRef) {

        auto transformer = GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&srcProjRef),
                                                               OGRSpatialReference::ToHandle(&dstProjRef), nullptr);
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