//
// Created by Jost on 10/09/2024.
//

#include <iostream>

#include "../mesh/interpolation.h"
#include "../mesh/mesh_converter.h"
#include "../mesh/mesh_operations.h"
#include "../mesh/raster_reader.h"
#include "../scene/TrackScene.h"

using TrackScene = TrackMapper::Scene::TrackScene;
using PointGrid = TrackMapper::Raster::PointGrid;
using OSMPoint = TrackMapper::Raster::OSMPoint;
using Point3D = TrackMapper::Raster::Point;


struct Config {
    TrackScene scene;
    std::string wkt;
    Point3D origin{};
    bool originIsSet = false;
    bool markersAreSet = false;

    std::vector<PointGrid> rasters;
};

void addTerrain(Config &config);
void addRoad(Config &config);
void writeOut(const Config &config);
OGRSpatialReference getValidSpatRef(Config &config);
std::vector<OSMPoint> readPathFromCSV(const std::string &filePath);

int main() {
    Config config;

    bool quit = false;
    while (!quit) {
        std::cout << "Select option: add [T]errain, add [R]oad, [E]xport, [Q]uit:" << std::endl;
        std::string selection;
        std::cin >> selection;

        switch (std::tolower(selection[0])) {
            case 'q':
                quit = true;
                break;
            case 't':
                addTerrain(config);
                break;
            case 'r':
                addRoad(config);
                break;
            case 'e':
                writeOut(config);
                quit = true;
                break;
            default:
                std::cout << "invalid option: " << selection[0] << std::endl;
                break;
        }

        std::cout << std::endl;
    }
}

void addTerrain(Config &config) {
    std::cout << "Enter path to geo raster file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::cout << "Task 1/4: Opening geo dataset" << std::endl;
    TrackMapper::Raster::GDALDatasetWrapper dataset(inFilePath);
    const auto pointGrid = TrackMapper::Raster::readRasterData(dataset);

    std::cout << "Task 2/4: Creating mesh" << std::endl;
    auto mesh = TrackMapper::Mesh::meshFromRasterData(pointGrid);

    std::cout << "Task 3/4: Simplifying mesh" << std::endl;
    const double reductionRation = std::min(40e3 / mesh.number_of_vertices(), 0.5);
    TrackMapper::Mesh::reduceMesh(mesh, reductionRation);

    std::cout << "Task 4/4: Adding mesh to scene" << std::endl;
    if (!config.originIsSet) {
        config.origin = pointGrid.origin;
        config.wkt = pointGrid.wkt;
        config.originIsSet = true;
    }

    const auto [x, y, z] = pointGrid.origin - config.origin;
    auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {x, y, z});

    config.scene.AddGrassMesh(sceneMesh);
    config.rasters.push_back(pointGrid);
    std::cout << "Finished: Added terrain with " << sceneMesh.vertices.size() << " vertices to scene" << std::endl;
}

void addRoad(Config &config) {
    if (!config.originIsSet) {
        std::cout << "Please first add at least one terrain befor adding any road" << std::endl;
        return;
    }

    std::cout << "Enter path to csv file containing the waypoints of the road:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::vector<OSMPoint> points = readPathFromCSV(inFilePath);

    OGRSpatialReference rasterSpatRef = getValidSpatRef(config);
    TrackMapper::Raster::reprojectOSMPoints(points, rasterSpatRef);

    // interpolate points for equal point density
    std::vector<Point3D> rawPoints;
    rawPoints.reserve(points.size());
    for (auto [x, y]: points) { // get point height
        // TODO: determine the correct raster the point lies in
        const auto currentRaster = config.rasters[0];
        Point3D p{x - currentRaster.origin.x, 0, y - currentRaster.origin.z};
        TrackMapper::Raster::interpolateHeightInGrid(currentRaster, p);
        rawPoints.push_back(p);
    }
    auto samples = TrackMapper::Mesh::interpolateCatmullRom(rawPoints, 1000);

    // update height of interpolated points
    TrackMapper::Mesh::Path path;
    path.points.reserve(points.size());
    for (auto p: samples) {
        const auto currentRaster = config.rasters[0];
        TrackMapper::Raster::interpolateHeightInGrid(currentRaster, p);

        // TODO: determine the correct raster the point lies in
        const auto [offsetX, offsetY, offsetZ] = config.origin - currentRaster.origin;
        path.points.emplace_back(p.x - offsetX, p.y - offsetY, p.z - offsetZ);
    }

    // set pit spawn point and start line (minimum of required markers for functioning map)
    if (!config.markersAreSet) {
        const auto pit = Point3D{path.points[3].x(), path.points[3].y() + 1, path.points[3].z()};
        const auto dir =
                Point3D{path.points[4].x(), 0, path.points[4].z()} - Point3D{path.points[3].x(), 0, path.points[3].z()};
        const auto [startX, startY, startZ] = pit + (3. / pit.Length()) * dir;

        // TODO: fix spawn orientation
        config.scene.AddSpawnPoint("AC_START_0", {pit.x, pit.y, pit.z}, {dir.x, 0, dir.z});
        config.scene.AddSpawnPoint("AC_PIT_0", {startX, startY, startZ}, {dir.x, 0, dir.z});

        config.markersAreSet = true;
    }

    // create mesh from path and add it to the scene
    // TODO: make width and subdivisions configurable
    const auto mesh = TrackMapper::Mesh::meshFromPath(path, 6, 5);
    // TODO: maybe set origin to position of first path node
    auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {0, 0, 0});

    config.scene.AddRoadMesh(sceneMesh);
}

void writeOut(const Config &config) {
    std::cout << "Enter path for exported fbx file:" << std::endl;
    std::string outFilePath;
    std::cin >> outFilePath;

    config.scene.Export(outFilePath, false);

    std::cout << "File successfully written to:\n" << outFilePath << std::endl;
}

OGRSpatialReference getValidSpatRef(Config &config) {
    OGRSpatialReference spatRef(config.wkt.c_str());

    while (spatRef.Validate() != OGRERR_NONE) {
        std::cout << "No valid projection reference could be determined!" << std::endl;
        std::cout << "Please specify projection reference manually by entering a valid OGC WKT:" << std::endl;
        std::string inProjRef;
        std::getline(std::cin >> std::ws, inProjRef);

        spatRef = OGRSpatialReference(inProjRef.c_str());
        config.wkt = spatRef.exportToWkt();
    }

    return spatRef;
}

std::vector<OSMPoint> readPathFromCSV(const std::string &filePath) {
    std::vector<OSMPoint> points;
    if (std::ifstream file(filePath); file.is_open()) {
        std::string line;
        while (file.good()) {
            std::getline(file, line);
            const size_t seperator = line.find_first_of(';');
            const double lat = std::stod(line.substr(0, seperator));
            const double lng = std::stod(line.substr(seperator + 1));
            points.emplace_back(lat, lng);
        }
        file.close();
    }

    return points;
}
