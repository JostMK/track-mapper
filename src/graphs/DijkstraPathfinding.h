//
// Created by Jost on 19/04/2024.
//

#ifndef DIJKSTRAPATHFINDING_H
#define DIJKSTRAPATHFINDING_H
#include <memory>

#include "IGraph.h"

struct Path {
    std::vector<int> path;
    int distance;
};

class DijkstraPathfinding {
public:
    explicit DijkstraPathfinding(const IGraph& graph);
    Path CalculatePath(int startNodeIndex, int targetNodeIndex);
private:
    const IGraph& graph;
    std::unique_ptr<int[]> predecessors;
    std::unique_ptr<int[]> distances;
};



#endif //DIJKSTRAPATHFINDING_H
