//
// Created by Jost on 22/09/2024.
//

#include "TrackCreator.h"

#include <utility>

namespace TrackMapper::Scene {

    // TODO: implement
    TrackCreator::TrackCreator(std::string name) : mName(std::move(name)) {}

    void TrackCreator::AddRaster(const std::string &filePath) {}

    void TrackCreator::AddPath(const std::vector<Raster::OSMPoint> &points, const Raster::ProjectionWrapper &projRef) {}

    void TrackCreator::AddSpawn(const Raster::Point &pit, const Raster::Point &direction) {}

    void TrackCreator::Export(const std::string &directoryPath) const {}

    // Todo: add closed path detection
} // namespace TrackMapper::Scene
