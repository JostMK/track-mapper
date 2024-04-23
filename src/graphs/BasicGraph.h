//
// Created by Jost on 21/04/2024.
//

#ifndef SIMPLEGRAPH_H
#define SIMPLEGRAPH_H
#include <memory>

#include "IGraph.h"


class BasicGraph final : IGraph {
public:
    BasicGraph(int nodeCount, int edgeCount, std::unique_ptr<Location[]> nodeLocations,
               std::unique_ptr<int[]> edgesLookupIndices, std::unique_ptr<Edge[]> edges);

    [[nodiscard]] int GetNodeCount() const override;

    [[nodiscard]] std::vector<Edge> GetEdges(int nodeIndex) const override;

    [[nodiscard]] Location GetLocation(int nodeIndex) const override;

private:
    int m_NodeCount;
    int m_EdgeCount;
    std::unique_ptr<Location[]> m_pNodeLocations;
    std::unique_ptr<int[]> m_pEdgesLookupIndices;
    std::unique_ptr<Edge[]> m_pEdges;
};


#endif //SIMPLEGRAPH_H
