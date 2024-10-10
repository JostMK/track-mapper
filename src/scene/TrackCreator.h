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

namespace TrackMapper::Mesh {
    struct Path;
}

namespace TrackMapper::Scene {

    class TrackCreator {
    public:
        explicit TrackCreator(std::string name);

        void AddRaster(const std::string &filePath);

        /// @note Make sure to first add all rasters so the path can get the correct height data
        void AddRoad(std::vector<Raster::OSMPoint> &points, const Raster::ProjectionWrapper &projRef, double width);

        /// Places Pit and Start spawn at p0 pointing towards p1
        /// @note Reprojects and adds the correct height to the position
        void AddSpawn(const Raster::OSMPoint &p0, const Raster::OSMPoint &p1, const Raster::ProjectionWrapper &projRef);

        void Export(const std::string &directoryPath) const;

    private:
        std::string mName;
        TrackScene mScene;
        std::vector<Raster::PointGrid> mGrids; // needed for path height data
        Raster::Point mOrigin{}; // overall 3D reference point
        bool mOriginSet = false;
        Raster::PointGrid *pLastGrid; // caches last grid for faster height finding

        void mAddRoad(const Mesh::Path &path, double width);
        void mInterpolateHeightForPoint(Raster::Point &point);
        void mSetHeightForPoint(Raster::Point &point);
    };

} // namespace TrackMapper::Scene

#endif // TRACKCREATOR_H
