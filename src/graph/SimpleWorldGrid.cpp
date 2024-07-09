//
// Created by Jost on 14/05/2024.
//

#include "SimpleWorldGrid.h"

#include <algorithm>
#include <array>
#include <cmath>

static Location clampLocation(const Location &location);

SimpleWorldGrid::SimpleWorldGrid(const IGraph &graph, const float resolution)
    : m_rGraph(graph),
      m_Resolution(resolution),
      m_CellCountX(std::floor(360 / resolution)),
      m_CellCountY(std::floor(180 / resolution)),
      m_pNodeIndices(std::make_unique<int[]>(graph.GetNodeCount())),
      m_pCellLookupIndices(std::make_unique<int[]>(m_CellCountX * m_CellCountY + 1)) {
    // -- fill array with indices for the nodes --
    std::vector<std::pair<int, int> > sorting;
    sorting.reserve(graph.GetNodeCount());
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        sorting.emplace_back(i, GetCellIndexForLocation(graph.GetLocation(i)));
    }

    // -- sort the node indices based on their cell index
    std::ranges::sort(sorting, [this, &graph](const std::pair<int, int> &left, const std::pair<int, int> &right) {
        return left.second < right.second;
    });
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        m_pNodeIndices[i] = sorting[i].first;
    }

    // -- iterate over all nodes now sorted by their cells and fill the offset table --
    int lastCellIndex = -1;

    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        if (const int cellIndex = sorting[i].second; lastCellIndex != cellIndex) {
            for (int j = lastCellIndex + 1; j <= cellIndex; j++) {
                m_pCellLookupIndices[j] = i;
            }
            lastCellIndex = cellIndex;
        }
    }
    for (int j = lastCellIndex + 1; j < m_CellCountX * m_CellCountY + 1; j++) {
        m_pCellLookupIndices[j] = graph.GetNodeCount();
    }
}

int SimpleWorldGrid::GetClosestNode(const Location location) const {
    // This algorithm fails close to the poles because of the convergence of the longitude lines
    //TODO: implement more sophisticated algorithm that does not fail
    // -> maybe not needed because users will most likely click close to the desired point anyway

    // checks all 9 cells in a rectangle around the location
    // uses GetCellIndexForLocation to automatically acount for crossing antimeridian
    const std::array cellsToCheck = {
        GetCellIndexForLocation({location.latitude - m_Resolution, location.longitude - m_Resolution}),
        GetCellIndexForLocation({location.latitude - m_Resolution, location.longitude}),
        GetCellIndexForLocation({location.latitude - m_Resolution, location.longitude + m_Resolution}),

        GetCellIndexForLocation({location.latitude, location.longitude - m_Resolution}),
        GetCellIndexForLocation(location),
        GetCellIndexForLocation({location.latitude, location.longitude + m_Resolution}),

        GetCellIndexForLocation({location.latitude + m_Resolution, location.longitude - m_Resolution}),
        GetCellIndexForLocation({location.latitude + m_Resolution, location.longitude}),
        GetCellIndexForLocation({location.latitude + m_Resolution, location.longitude + m_Resolution}),
    };

    double minDist = std::numeric_limits<double>::max();
    int minDistNodeIndex = -1;

    for (const int cellIndex : cellsToCheck) {
        for (const int nodeIndex: GetNodeIndicesInCell(cellIndex)) {
            auto [nodeLatitude, nodeLongitude] = m_rGraph.GetLocation(nodeIndex);
            const double sqrDist = std::pow(location.latitude - nodeLatitude, 2) +
                                   std::pow(location.longitude - nodeLongitude, 2);

            if (sqrDist < minDist) {
                minDist = sqrDist;
                minDistNodeIndex = nodeIndex;
            }
        }
    }

    return minDistNodeIndex;
}

int SimpleWorldGrid::GetCellIndexForLocation(const Location &location) const {
    const auto [latitude, longitude] = clampLocation(location);
    const int xIndex = std::floor((latitude + 90) / m_Resolution);
    const int yIndex = std::floor((longitude + 180) / m_Resolution);

    return xIndex * m_CellCountY + yIndex;
}

std::vector<int> SimpleWorldGrid::GetNodeIndicesInCell(const int cellIndex) const {
    const int startIndex = m_pCellLookupIndices[cellIndex];
    const int nextNodeStartIndex = m_pCellLookupIndices[cellIndex + 1];

    std::vector<int> indices;
    indices.reserve(nextNodeStartIndex - startIndex);
    for (int i = startIndex; i < nextNodeStartIndex; ++i) {
        indices.push_back(m_pNodeIndices[i]);
    }

    return indices;
}

/**
 * Clamps latitude to [-89,89] and longitude to [-180, 180)
 * @param location reference to the location that gets clamped
 */
static Location clampLocation(const Location &location) {
    // since the input comes from a map latitude outside of the interval dont make sense and just get clamped
    // the simple grid brakes at the poles so clamping latitude to [-89, 89]
    const double lat = std::clamp(location.latitude, -89., 89.);

    const double lon = std::fmod(std::fmod(location.longitude + 180., 360.) + 360., 360.) - 180.;

    return Location{lat, lon};
}
