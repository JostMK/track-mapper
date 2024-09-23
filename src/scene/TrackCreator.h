//
// Created by Jost on 22/09/2024.
//

#ifndef TRACKCREATOR_H
#define TRACKCREATOR_H
#include <string>
#include <vector>

#include "../mesh/gdal_wrapper.h"
#include "../mesh/raster_reader.h"

#include "TrackScene.h"

namespace TrackMapper::Scene {

    class TrackCreator {
    public:
        explicit TrackCreator(std::string name);

        void AddRaster(const std::string &filePath);

        /// @note Make sure to first add all rasters so the path can get the correct height data
        void AddRoad(const std::vector<Raster::OSMPoint> &points, const Raster::ProjectionWrapper &projRef);

        /// @note Adds the correct height to the position
        void AddSpawn(const Raster::Point &pit, const Raster::Point &direction);

        void Export(const std::string &directoryPath) const;

    private:
        std::string mName;
        TrackScene mScene;
        std::vector<Raster::PointGrid> rasters; // needed for path height data
        Raster::Point origin{}; // overall 3D reference point
    };

} // namespace TrackMapper::Scene

#endif // TRACKCREATOR_H
