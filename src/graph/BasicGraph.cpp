//
// Created by Jost on 21/04/2024.
//

#include "BasicGraph.h"

BasicGraph::BasicGraph(const int nodeCount,
                       const int edgeCount,
                       std::unique_ptr<Location[]> nodeLocations,
                       std::unique_ptr<int[]> edgesLookupIndices,
                       std::unique_ptr<Edge[]> edges)
    : m_NodeCount(nodeCount),
      m_EdgeCount(edgeCount),
      m_pNodeLocations(std::move(nodeLocations)),
      m_pEdgesLookupIndices(std::move(edgesLookupIndices)),
      m_pEdges(std::move(edges)) {
}

int BasicGraph::GetNodeCount() const {
    return m_NodeCount;
}

std::vector<Edge> BasicGraph::GetEdges(const int nodeIndex) const {
    const int startIndex = m_pEdgesLookupIndices[nodeIndex];
    const int nextNodeStartIndex = m_pEdgesLookupIndices[nodeIndex+1];

    std::vector<Edge> edges;
    edges.reserve(nextNodeStartIndex-startIndex);
    for (int i = startIndex; i < nextNodeStartIndex; ++i) {
        edges.push_back(m_pEdges[i]);
    }

    return edges;
}

Location BasicGraph::GetLocation(const int nodeIndex) const {
    return m_pNodeLocations[nodeIndex];
}


