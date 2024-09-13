//
// Created by Jost on 13/09/2024.
//

#ifndef INTERPOLATION_H
#define INTERPOLATION_H
#include <vector>

#include "raster_reader.h"

namespace TrackMapper::Mesh {

    std::vector<Raster::Point> interpolateCatmullRom(const std::vector<Raster::Point> &points, int sampleCount = 100,
                                                     float alpha = .5f);
} // namespace TrackMapper::Mesh

#endif // INTERPOLATION_H
