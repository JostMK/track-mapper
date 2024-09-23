//
// Created by Jost on 22/09/2024.
//

#include "TrackCreator.h"

#include <filesystem>
#include <iostream>
#include <utility>

#include "../mesh/gdal_wrapper.h"
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

    void set_height_for_point(const std::vector<Raster::PointGrid> &mRasters, Point3D &point);

    TrackCreator::TrackCreator(std::string name) : mName(std::move(name)) {}

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
        auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {x, y, z});

        mScene.AddGrassMesh(sceneMesh);
        mGrids.push_back(pointGrid);
    }


    void TrackCreator::AddRoad(std::vector<Raster::OSMPoint> &points, const Raster::ProjectionWrapper &projRef) {
        if (!mOriginSet) {
            std::cout << "Not possible to add road without adding at least one raster! (This shouldn't be happening)"
                      << std::endl;
            return;
        }

        // TODO: add closed path detection

        // reprojecting points into raster space
        TrackMapper::Raster::reprojectOSMPoints(points, projRef);

        // place points for mesh
        TrackMapper::Mesh::Path path;
        path.points.reserve(points.size());
        for (auto [x, y]: points) {
            Point3D p{x, 0, y};
            set_height_for_point(mGrids, p);
            auto [lX, lY, lZ] = p - mOrigin;
            path.points.emplace_back(lX, lY, lZ);
        }

        // create mesh from path
        // TODO: allow to change width
        // TODO: slice mesh to uphold vertex count limit
        // TODO: modify terrain to be below road
        // TODO: give road a vertical profile
        // TODO: add grass mesh near road ("grass road")
        const auto mesh = TrackMapper::Mesh::meshFromPath(path, 6, 5);

        // adding mesh to scene
        // Todo: make path origin sit at first node
        auto sceneMesh = TrackMapper::Mesh::cgalToSceneMesh(mesh, {0, 0, 0});
        mScene.AddRoadMesh(sceneMesh);
    }

    void TrackCreator::AddSpawn(const Raster::OSMPoint &p0, const Raster::OSMPoint &p1,
                                const Raster::ProjectionWrapper &projRef) {
        // reprojecting points into raster space
        std::vector points{p0, p1};
        TrackMapper::Raster::reprojectOSMPoints(points, projRef);

        // calculate pit spawn position and direction
        auto pit = Point3D{p0.lat, 0, p0.lng}; // height will be set in TrackCreator
        const auto [dirX, dirY, dirZ] = Point3D{p1.lat, 0, p1.lng} - pit;

        // get correct height for spawns
        set_height_for_point(mGrids, pit);

        // add marker object to scene
        mScene.AddSpawnPoint("AC_PIT_0", {pit.x, pit.y + 1, pit.z}, {dirX, dirY, dirZ});
        mScene.AddSpawnPoint("AC_START_0", {pit.x, pit.y + 1, pit.z}, {dirX, dirY, dirZ});
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

        // TODO: export raw version for further editing
    }

    void set_height_for_point(const std::vector<Raster::PointGrid> &mRasters, Point3D &point) {
        // default to 0
        point.y = 0;

        // find correct raster for point
        for (const auto &grid: mRasters) {
            auto offsetIntoRaster = point - grid.origin;
            if (offsetIntoRaster.x < 0 || offsetIntoRaster.z < 0)
                continue; // point is outside of raster

            Point3D farCorner = Point3D{static_cast<double>(grid.sizeX), 0, static_cast<double>(grid.sizeY)}.Transform(
                    grid.transform); // margin of one pixel because of floor

            // ReSharper disable once CppUseStructuredBinding
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto offsetFarCorner = point - farCorner;
            if (offsetFarCorner.x >= 0 || offsetFarCorner.z >= 0)
                continue; // point is outside of raster

            Raster::interpolateHeightInGrid(grid, offsetIntoRaster);
            point.y = offsetIntoRaster.y;
            break;
        }
    }

} // namespace TrackMapper::Scene
