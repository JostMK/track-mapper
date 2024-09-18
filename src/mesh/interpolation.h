//
// Created by Jost on 13/09/2024.
//

#ifndef INTERPOLATION_H
#define INTERPOLATION_H
#include <vector>

#include "raster_reader.h"

namespace TrackMapper::Mesh {

    std::vector<Raster::Point> interpolateCatmullRom(const std::vector<Raster::Point> &points,
                                                     double sampleDistance = 0.2, float alpha = .5f);

    std::vector<Raster::Point> subdivideCatmullRom(const std::vector<Raster::Point> &points, int pointsPerSegment = 4,
                                                   float alpha = .5f);
} // namespace TrackMapper::Mesh

#endif // INTERPOLATION_H
