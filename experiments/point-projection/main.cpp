//
// Created by Jost on 01/08/2024.
//

#include <array>
#include <gdal_alg.h>
#include <gdal_priv.h>
#include <iostream>

struct Point2D {
    double x, y;
};

void rasterToWorld(const GDALDatasetUniquePtr &pDataset);
void osmToWorld(const GDALDatasetUniquePtr &pDataset);

int main() {
    CPLSetConfigOption("PROJ_LIB", "./proj");
    GDALAllRegister();

    std::cout << "Enter Path to geo raster file for Spatial Reference:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    const auto pDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(inFilePath.c_str(), GA_ReadOnly)));

    bool quit = false;
    while (!quit) {
        std::cout << "Select transform option: raster point to world point (r), osm point to world point (o), quit (q):"
                  << std::endl;
        std::cout << "[world points are in the projection of the raster file]:" << std::endl;
        std::string selection;
        std::cin >> selection;

        switch (selection[0]) {
            case 'q':
                quit = true;
                break;
            case 'r':
                rasterToWorld(pDataset);
                break;
            case 'o':
                osmToWorld(pDataset);
                break;
            default:
                std::cout << "invalid option: " << selection[0] << std::endl;
                break;
        }

        std::cout << std::endl;
    }

    pDataset->Close();
    return 0;
}

void rasterToWorld(const GDALDatasetUniquePtr &pDataset) {

    std::cout << "\nEnter Point:" << std::endl;
    std::cout << "x:";
    double x;
    std::cin >> x;
    std::cout << "y:";
    double y;
    std::cin >> y;

    std::array<double, 6> transform{};
    pDataset->GetGeoTransform(transform.data());

    Point2D p{transform[0] + x * transform[1] + y * transform[2], transform[3] + x * transform[4] + y * transform[5]};

    std::cout << "\nRaster Point to World Point: " << std::endl;
    std::cout << "(" << x << " : " << y << ") -> (" << p.x << " : " << p.y << ")" << std::endl;
}

void osmToWorld(const GDALDatasetUniquePtr &pDataset) {
    std::cout << "\nEnter Point:" << std::endl;
    std::cout << "lat:";
    double x;
    std::cin >> x;
    std::cout << "lng:";
    double y;
    std::cin >> y;

    Point2D p{x, y};

    OGRSpatialReference osmPointsSpatRef(
            R"(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])");

    std::string prjRef = pDataset->GetProjectionRef();

    if (prjRef.empty()) {
        std::cout << "\nNo Projection Reference found!" << std::endl;
        std::cout << "Enter Projection Reference manually:" << std::endl;
        std::getline(std::cin >> std::ws, prjRef);
    }

    OGRSpatialReference rasterSpatRef(prjRef.c_str());

    const auto pTransform =
                GDALCreateReprojectionTransformerEx(OGRSpatialReference::ToHandle(&osmPointsSpatRef),
                                                    OGRSpatialReference::ToHandle(&rasterSpatRef), nullptr);

    if (pTransform == nullptr) {
        std::cerr << "Error creating transformer!" << std::endl;
        return;
    }

    bool success = GDALReprojectionTransform(pTransform, 0, 1, &p.x, &p.y, nullptr, nullptr);

    if (!success) {
        std::cerr << "Error reprojecting!" << std::endl;
        return;
    }

    GDALDestroyReprojectionTransformer(pTransform);

    std::cout << "\nOSM Point to World Point: " << std::endl;
    std::cout << "(" << x << " : " << y << ") -> (" << p.x << " : " << p.y << ")" << std::endl;
}
