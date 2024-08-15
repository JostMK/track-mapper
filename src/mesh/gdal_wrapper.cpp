//
// Created by Jost on 12/08/2024.
//

#include "gdal_wrapper.h"

#include <gdal_alg.h>
#include <gdal_priv.h>

namespace TrackMapper::Raster {

    // ----- GDALProjectionReferenceWrapper -----

    struct GDALProjectionReferenceWrapper::impl {
        const OGRSpatialReference spatialRef;

        explicit GDALProjectionReferenceWrapper::impl(const std::string &wktString) : spatialRef(wktString.c_str()) {}
    };

    GDALProjectionReferenceWrapper::GDALProjectionReferenceWrapper(const std::string &wktString) :
        pImpl{std::make_unique<impl>(wktString)} {}
    GDALProjectionReferenceWrapper::~GDALProjectionReferenceWrapper() = default;

    GDALProjectionReferenceWrapper::GDALProjectionReferenceWrapper(const GDALProjectionReferenceWrapper &other) :
        pImpl{std::make_unique<impl>(other.GetWkt())} {}

    GDALProjectionReferenceWrapper::GDALProjectionReferenceWrapper(GDALProjectionReferenceWrapper &&other) noexcept :
        pImpl(std::exchange(other.pImpl, nullptr)) {}

    GDALProjectionReferenceWrapper &
    GDALProjectionReferenceWrapper::operator=(const GDALProjectionReferenceWrapper &other) {
        return *this = GDALProjectionReferenceWrapper(other);
    }

    GDALProjectionReferenceWrapper &
    GDALProjectionReferenceWrapper::operator=(GDALProjectionReferenceWrapper &&other) noexcept {
        std::swap(pImpl, other.pImpl);
        return *this;
    }

    std::string GDALProjectionReferenceWrapper::GetWkt() const { return pImpl->spatialRef.exportToWkt(); }

    bool GDALProjectionReferenceWrapper::IsValid() const { return pImpl->spatialRef.Validate() == OGRERR_NONE; }


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
        mProjRef = GDALProjectionReferenceWrapper(pDataset->GetProjectionRef());

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

    const GDALProjectionReferenceWrapper &GDALDatasetWrapper::GetProjectionRef() const { return mProjRef; }


    // ----- GDALReprojectionTransformer -----

    struct GDALReprojectionTransformer::impl {
        void *transformer; // GDALCreateReprojectionTransformer

        explicit GDALDatasetWrapper::impl(void *transformer) : transformer(transformer) {}
    };

    GDALReprojectionTransformer::GDALReprojectionTransformer(const GDALProjectionReferenceWrapper &srcProjRef,
                                                             const GDALProjectionReferenceWrapper &dstProjRef) {
        // auto transformer = GDALCreateReprojectionTransformer(srcProjRef.GetWkt().c_str(),
        // dstProjRef.GetWkt().c_str());
        OGRSpatialReference osmPointsSpatRef(srcProjRef.GetWkt().c_str());
        OGRSpatialReference rasterSpatRef(dstProjRef.GetWkt().c_str());
        auto transformer = GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&osmPointsSpatRef),
                                                               OGRSpatialReference::ToHandle(&rasterSpatRef), nullptr);
        pImpl = std::make_unique<impl>(transformer);
    }

    GDALReprojectionTransformer::~GDALReprojectionTransformer() {
        GDALDestroyReprojectionTransformer(pImpl->transformer);
    }

    bool GDALReprojectionTransformer::Transform(double *x, double *y, double *z) const {
        // if (pImpl->transformer == nullptr)
        //     return false;


        const std::string wktOSM =
                R"(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])";
        const std::string wktEU =
                R"(PROJCS["ETRS89 / UTM zone 32N",GEOGCS["ETRS89",DATUM["European_Terrestrial_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6258"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4258"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",9],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","25832"]])";

        const OGRSpatialReference osmPointsSpatRef(wktOSM.c_str());
        const OGRSpatialReference euPointsSpatRef(wktEU.c_str());

        OGRSpatialReference osmPointsSpatRefCopy(osmPointsSpatRef.exportToWkt().c_str());
        OGRSpatialReference euPointsSpatRefCopy(euPointsSpatRef.exportToWkt().c_str());

        const auto pTransform =
                GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&osmPointsSpatRefCopy),
                                                    OGRSpatialReference::ToHandle(&euPointsSpatRefCopy), nullptr);

        double lat = 48.54347;
        double lng = 9.396605;

        return GDALReprojectionTransform(pTransform, 0, 1, &lat, &lng, nullptr, nullptr);
    }

    bool GDALReprojectionTransformer::IsValid() const { return pImpl->transformer != nullptr; }

} // namespace TrackMapper::Raster
