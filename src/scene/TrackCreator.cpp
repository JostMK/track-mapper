//
// Created by Jost on 22/09/2024.
//

#include "TrackCreator.h"

#include <filesystem>
#include <iostream>
#include <utility>

#include "../mesh/gdal_wrapper.h"
#include "../mesh/interpolation.h"
#include "../mesh/mesh_converter.h"
#include "../mesh/mesh_operations.h"
#include "../mesh/raster_reader.h"
#include "../scene/TrackScene.h"

using ProjectionWrapper = TrackMapper::Raster::ProjectionWrapper;
using TrackScene = TrackMapper::Scene::TrackScene;
using PointGrid = TrackMapper::Raster::PointGrid;
using OSMPoint = TrackMapper::Raster::OSMPoint;
using Point3D = TrackMapper::Raster::Point;

namespace TrackMapper::Scene {
    void set_height_for_point(const Raster::PointGrid &grid, Point3D &point);
    bool is_point_in_grid(const Raster::PointGrid &grid, const Point3D &point);

    double lerp(const double a, const double b, const double t) { return a + (b - a) * t; }

    TrackCreator::TrackCreator(std::string name) : mName(std::move(name)), pLastGrid(nullptr) {}

    void TrackCreator::AddRaster(const std::string &filePath) {
        // opening dataset
        TrackMapper::Raster::GDALDatasetWrapper dataset(filePath);
        if (!dataset.IsValid()) {
            // Todo: handle - should only be possible if file gets deleted in the meantime
            std::cout << "The dataset for '" << filePath << "' could not be opened! (This shouldn't be happening)"
                      << std::endl;
            return;
        }
        const auto pointGrid = TrackMapper::Raster::readRasterData(dataset);

        // TODO: Add tile slicing for resolution control and performance

        // creating mesh
        auto mesh = TrackMapper::Mesh::meshFromRasterData(pointGrid);

        // simplifying mesh
        const double reductionRation = std::min(40e3 / mesh.number_of_vertices(), 0.5);
        TrackMapper::Mesh::reduceMesh(mesh, reductionRation);

        // adding mesh to scene
        if (!mOriginSet) {
            mOrigin = pointGrid.origin;
            mOriginSet = true;
        }

        const auto [x, y, z] = pointGrid.origin - mOrigin;
        auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {x, y, z}, [&mesh](const int i) -> Scene::Double2 {
            const auto point = mesh.point(static_cast<CGAL::SM_Vertex_index>(i));
            return {point.x(), point.z()}; // projects texture from above with one full texture per square meter
        });

        mScene.AddGrassMesh(sceneMesh);
        mGrids.push_back(pointGrid);
    }


    void TrackCreator::AddRoad(std::vector<Raster::OSMPoint> &points, const Raster::ProjectionWrapper &projRef,
                               const double width) {
        if (!mOriginSet) {
            std::cout << "Not possible to add road without adding at least one raster! (This shouldn't be happening)"
                      << std::endl;
            return;
        }

        // TODO: add closed path detection

        // reprojecting points into raster space
        TrackMapper::Raster::reprojectOSMPoints(points, projRef);

        // interpolate path
        std::vector<Point3D> rawPoints;
        rawPoints.reserve(points.size());
        for (auto [x, y]: points) {
            Point3D p{x, 0, y};
            mSetHeightForPoint(p); // rough height is ok because it gets corrected later
            rawPoints.push_back(p);
        }

        auto samples = Mesh::subdivideCatmullRom(rawPoints, 4);

        // update height of interpolated points
        for (auto &p: samples) {
            mInterpolateHeightForPoint(p);
        }

        samples = Mesh::interpolateCatmullRom(rawPoints, 0.5); // approx. 0.5m between vertices

        // TODO: modify terrain to be below road
        // TODO: add grass mesh near road ("grass road")

        // slice path to uphold vertex count limit of 40k
        const int widthSubdivisionCount = static_cast<int>(std::ceil(width * 2)) + 1; // approx. 0.5m between vertices
        const int vertexCount = static_cast<int>(widthSubdivisionCount * (samples.size() + 1));
        const double sliceCount = std::ceil(vertexCount / 40e3);
        const int pointsPerSlice =
                static_cast<int>(std::ceil(static_cast<double>(samples.size() + 1) / sliceCount)) + 1;

        TrackMapper::Mesh::Path path;
        path.points.reserve(pointsPerSlice);

        // add a dummy point at start and end for direction calculation
        const auto start = samples[0] * 2 - samples[1];
        const auto end = samples[samples.size() - 1] * 2 - samples[samples.size() - 2];
        samples.push_back(end);
        auto [sX, sY, sZ] = start - mOrigin;
        path.points.emplace_back(sX, sY, -sZ);

        for (int i = 0; i < samples.size() - 1; ++i) {
            // place points for mesh
            auto [x, y, z] = samples[i] - mOrigin;
            path.points.emplace_back(x, y, -z); // Note: fbx coordinate system needs z mirroring

            // create mesh and add it to scene
            if (path.points.size() >= pointsPerSlice) {
                // add next point because mesh creation needs it for the direction calculation
                auto [nX, nY, nZ] = samples[i + 1] - mOrigin;
                path.points.emplace_back(nX, nY, -nZ);

                mAddRoad(path, width);
                path.points.clear();

                // add last point for next slice because mesh creation needs it for the direction calculation
                auto [pX, pY, pZ] = samples[i - 1] - mOrigin;
                path.points.emplace_back(pX, pY, -pZ);
                path.points.emplace_back(x, y, -z);
            }
        }
        // if the points can not be perfectly distributed into the slices the last slice falls short of pointsPerSlice
        // and needs to be submitted separately
        if (path.points.size() > 1) {
            // add next point because mesh creation needs it for the direction calculation
            auto [nX, nY, nZ] = samples[samples.size() - 1] - mOrigin;
            path.points.emplace_back(nX, nY, -nZ);

            mAddRoad(path, width);
        }
    }

    void TrackCreator::AddSpawn(const Raster::OSMPoint &p0, const Raster::OSMPoint &p1,
                                const Raster::ProjectionWrapper &projRef) {
        // reprojecting points into raster space
        std::vector points{p0, p1};
        TrackMapper::Raster::reprojectOSMPoints(points, projRef);

        // calculate pit spawn position and direction
        auto pit = Point3D{p0.lat, 0, p0.lng};
        const auto [dirX, dirY, dirZ] = Point3D{p1.lat, 0, p1.lng} - pit;

        // get correct height for spawns
        mInterpolateHeightForPoint(pit);
        pit -= mOrigin;

        // add marker object to scene
        // Note: fbx coordinate system needs z mirroring
        mScene.AddSpawnPoint("AC_PIT_0", {pit.x, pit.y + 1, -pit.z}, {dirX, dirY, -dirZ});
        mScene.AddSpawnPoint("AC_START_0", {pit.x, pit.y + 1, -pit.z}, {dirX, dirY, -dirZ});
    }

    void TrackCreator::Export(const std::string &directoryPath) const {
        const std::filesystem::path dir{directoryPath};

        if (std::error_code ec; !std::filesystem::is_directory(dir, ec)) {
            std::cout << "Provided export path '" << directoryPath
                      << "' is not a directory! (This shouldn't be happening)" << std::endl;
            return;
        }

        // create correct mod file structure if not existent
        // TODO: create folder structure

        // export track file ready for importing into ksEditor
        const std::filesystem::path trackFBX = dir / (mName + ".fbx");
        mScene.Export(trackFBX.string(), true);

        const std::filesystem::path fullFBX = dir / (mName + "-raw.fbx");
        mScene.Export(fullFBX.string(), false);

        // TODO: export raw version for further editing
    }

    void TrackCreator::mAddRoad(const Mesh::Path &path, const double width) {
        if (path.points.size() < 4) {
            std::cout << "Provided road path needs to have at least 4 points! (This shouldn't be happening)"
                      << std::endl;
            return;
        }

        // create mesh from path
        // TODO: give road a vertical profile
        const int widthSubdivisionCount = static_cast<int>(std::ceil(width * 2)) + 1; // approx. 0.5m between vertices
        const auto mesh = TrackMapper::Mesh::meshFromPath(path, width, widthSubdivisionCount);

        // Note: assumes points are equally spaced
        const double vScaling = std::sqrt(CGAL::squared_distance(path.points[0], path.points[1])) / width;

        // adding mesh to scene
        // Todo: make path origin sit at first node
        auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(
                mesh, {0, 0, 0}, [widthSubdivisionCount, vScaling](const int i) -> Scene::Double2 {
                    const double u = (i % widthSubdivisionCount) / static_cast<double>(widthSubdivisionCount - 1);

                    // Note: assumes points are equally spaced
                    // Note: integer devision is intended
                    const double v = (i / widthSubdivisionCount) * vScaling; // NOLINT(*-integer-division)
                    return {u, v}; // projects texture along path with one full texture per width*width area
                });
        mScene.AddRoadMesh(sceneMesh);
    }

    void TrackCreator::mInterpolateHeightForPoint(Raster::Point &point) {
        if (mGrids.empty()) {
            point.y = 0;
            return;
        }

        // get sampling resolution for height data
        // Note: only supports rasters with uniform solution
        const double pixelSizeX = mGrids[0].pixelSizeX;
        const double pixelSizeZ = mGrids[0].pixelSizeY;

        // bilinear interpolate height
        const Point3D center{std::round(point.x), 0, std::round(point.z)};

        Point3D pTL{center.x - pixelSizeX, 0, center.z - pixelSizeZ};
        mSetHeightForPoint(pTL);

        Point3D pTR{center.x, 0, center.z - pixelSizeZ};
        mSetHeightForPoint(pTR);

        Point3D pBL{center.x - pixelSizeX, 0, center.z};
        mSetHeightForPoint(pBL);

        Point3D pBR{center.x, 0, center.z};
        mSetHeightForPoint(pBR);

        const auto [offsetX, offsetY, offsetZ] = point - center;

        const auto h1 = lerp(pTL.y, pTR.y, pixelSizeX * .5 + offsetX);
        const auto h2 = lerp(pBL.y, pBR.y, pixelSizeX * .5 + offsetX);

        point.y = lerp(h1, h2, pixelSizeZ * .5 + offsetZ);
    }

    void TrackCreator::mSetHeightForPoint(Raster::Point &point) {
        // default to 0
        point.y = 0;

        // if point is cached grid directly set height
        if (pLastGrid != nullptr && is_point_in_grid(*pLastGrid, point)) {
            const auto offsetIntoRaster = point - pLastGrid->origin;
            point.y = Raster::GetHeightForPointInGrid(*pLastGrid, offsetIntoRaster);
            return;
        }

        // if not find correct raster for point
        for (auto &grid: mGrids) {
            if (!is_point_in_grid(grid, point))
                continue;

            const auto offsetIntoRaster = point - grid.origin;
            point.y = Raster::GetHeightForPointInGrid(grid, offsetIntoRaster);
            pLastGrid = &grid;
            break;
        }
    }

    void set_height_for_point(const Raster::PointGrid &grid, Point3D &point) {
        const auto offsetIntoRaster = point - grid.origin;
        point.y = Raster::GetHeightForPointInGrid(grid, offsetIntoRaster);
    }

    bool is_point_in_grid(const Raster::PointGrid &grid, const Point3D &point) {
        // offset to near corner
        // ReSharper disable once CppTooWideScopeInitStatement
        auto [offsetX, offsetY, offsetZ] = point - grid.origin;
        if (offsetX < 0 || offsetZ < 0)
            return false;

        Point3D farCorner; // dependent on transform orientation
        if (grid.transform[5] < 0) {
            farCorner = Raster::getRasterPoint(grid.transform, grid.sizeX, -1, true);
        } else {
            farCorner = Raster::getRasterPoint(grid.transform, grid.sizeX, grid.sizeY, true);
        }

        // offset to far corner
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto [offsetFCX, offsetFCY, offsetFCZ] = point - farCorner;
        if (offsetFCX >= 0 || offsetFCZ >= 0)
            return false;

        return true;
    }

} // namespace TrackMapper::Scene
