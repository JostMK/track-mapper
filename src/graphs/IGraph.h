//
// Created by Jost on 16/04/2024.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

const struct Location {
    double latitude;
    double longitude;
};

const struct Edge {
    int adjacentNodeIndex;
    int distance;
};

class IGraph {
public:
    virtual ~IGraph() = default;

    [[nodiscard]] virtual int GetNodeCount() const = 0;

    [[nodiscard]] virtual std::vector<Edge> GetEdges(int nodeIndex) const = 0;

    [[nodiscard]] virtual Location GetLocation(int nodeIndex) const = 0;
};

#endif //GRAPH_H
