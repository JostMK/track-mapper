//
// Created by Jost on 14/05/2024.
//

#include "SimpleWorldGrid.h"

#include <algorithm>
#include <cmath>

SimpleWorldGrid::SimpleWorldGrid(const IGraph &graph, const float resolution)
    : m_Resolution(resolution),
      m_CellCountX(std::floor(360 / resolution)),
      m_CellCountY(std::floor(180 / resolution)),
      m_pNodeIndices(std::make_unique<int[]>(graph.GetNodeCount())),
      m_pCellLookupIndices(std::make_unique<int[]>(m_CellCountX * m_CellCountY + 1)) {
    // -- fill array with indices for the nodes --
    std::vector<std::pair<int,int>> sorting;
    sorting.reserve(graph.GetNodeCount());
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        sorting.emplace_back(i, GetCellIndexForLocation(graph.GetLocation(i)));
    }

    // -- sort the node indices based on their cell index
    std::ranges::sort(sorting, [this, &graph](const std::pair<int,int>& left, const std::pair<int,int>& right) {
        return left.second < right.second;
    });
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        m_pNodeIndices[i] = sorting[i].first;
    }

    // -- iterate over all nodes now sorted by thier cells and fill the offset table --
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

int SimpleWorldGrid::GetClosestNode(Location location) const {
    // TODO: implement
    return 0;
}

int SimpleWorldGrid::GetCellIndexForLocation(const Location &location) const {
    const int xIndex = std::floor((location.latitude + 90) / m_Resolution);
    const int yIndex = std::floor((location.longitude + 180) / m_Resolution);

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
