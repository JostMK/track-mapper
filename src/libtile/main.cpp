//
// Created by Jost on 19/09/2025.
//

#include <iostream>

#include "../libscene/Mesh.h"
#include "GeoTile.h"

LibScene::Mesh create_mesh_from_tile(const LibTile::GeoTile &tile) {
    std::vector<LibScene::Double3> vertices;
    std::vector<LibScene::Face> faces;

    LibScene::Double3 origin;
    tile.geo_transform.get_origin(origin.x, origin.y);

    vertices.reserve(tile.height.size());
    for (int y = 0; y < tile.size_y; ++y) {
        for (int x = 0; x < tile.size_x; ++x) {
            double transform_x = x;
            double transform_y = y;
            tile.geo_transform.transform_relative(transform_x, transform_y);

            const int index = y * tile.size_x + x;
            double height = tile.height[index];

            // note: flip z axis because the image real world coordinates grow towards nord (upwards in the image)
            //       the image coordinates however grow downwards which is mapped onto the z axis.
            vertices.emplace_back(transform_x, height, -transform_y);
        }
    }

    for (int y = 0; y < tile.size_y - 1; ++y) {
        for (int x = 0; x < tile.size_x - 1; ++x) {
            const int indexTL = y * tile.size_x + x;
            const int indexTR = y * tile.size_x + (x + 1);
            const int indexBL = (y + 1) * tile.size_x + x;
            const int indexBR = (y + 1) * tile.size_x + (x + 1);

            faces.emplace_back(indexTL, indexTR, indexBL);
            faces.emplace_back(indexTR, indexBR, indexBL);
        }
    }

    return {{origin.x, 0, -origin.z}, std::move(vertices), std::move(faces)};
}

bool create_mesh_from_file(const std::string &inFilepath, const std::string &outFilepath) {
    std::cout << "> Loading file.. (" << inFilepath << ")" << std::endl;

    const auto expectedTile = LibTile::load_from_file(inFilepath);

    if (!expectedTile.has_value()) {
        switch (expectedTile.error()) {
            case LibTile::Error::FAILED_TO_OPEN_DATASET:
                std::cout << "FAILED_TO_OPEN_DATASET" << std::endl;
                break;
            case LibTile::Error::FAILED_TO_GET_GEO_TRANSFORM:
                std::cout << "FAILED_TO_GET_GEO_TRANSFORM" << std::endl;
                break;
            case LibTile::Error::FAILED_TO_READ_DATA:
                std::cout << "FAILED_TO_READ_DATA" << std::endl;
                break;
            default:
                std::cout << "UNKNOWN ERROR" << std::endl;
                break;
        }
        return false;
    }

    const auto &tile = expectedTile.value();

    std::cout << "Tile size: " << tile.size_x << " x " << tile.size_y << std::endl;
    std::cout << "Tile proj: " << tile.proj_wkt << std::endl;
    std::cout << std::endl;

    std::cout << "> Creating mesh.." << std::endl;

    const auto mesh = create_mesh_from_tile(tile);

    std::cout << "Vertex count: " << mesh.vertices.size() << std::endl;
    std::cout << "Face count: " << mesh.faces.size() << std::endl;
    std::cout << "Origin: " << mesh.origin.x << " : " << mesh.origin.y << " : " << mesh.origin.z << std::endl;
    std::cout << std::endl;

    std::cout << "> Exporting mesh.." << std::endl;

    if (const auto export_error = mesh.export_to_obj(outFilepath)) {
        switch (export_error) {
            case LibScene::Error::FAILED_TO_OPEN_OUTPUT_FILE:
                std::cout << "FAILED_TO_OPEN_OUTPUT_FILE" << std::endl;
                break;
            default:
                std::cout << "UNKNOWN ERROR" << std::endl;
                break;
        }
        return false;
    }

    std::cout << "Output file at '" << outFilepath << "'" << std::endl;
    return true;
}

// namespace LibScene
int main(int argc, char **argv) {
    create_mesh_from_file(
            R"(D:\Documents\GitHub\track-mapper\data\raster-file\road-creation-test\mv\dgm1_33_320_5984_2_gtiff.tif)",
            "Mesh2.obj");
    std::cout << std::endl;

    create_mesh_from_file(
            R"(D:\Documents\GitHub\track-mapper\data\raster-file\road-creation-test\bw\dgm1_32_529_5376_1_bw_2017.xyz)",
            "Mesh3.obj");
}
