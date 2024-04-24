//
// Created by Jost on 19/04/2024.
//

#include "DijkstraPathfinding.h"

#include <limits>
#include <queue>

DijkstraPathfinding::DijkstraPathfinding(const IGraph &graph) : graph(graph) {
    predecessors = std::make_unique<int[]>(graph.GetNodeCount());
    distances = std::make_unique<int[]>(graph.GetNodeCount());
}

Path DijkstraPathfinding::CalculatePath(int startNodeIndex, int targetNodeIndex) {
    //std::fill_n(&predecessors, graph.GetNodeCount(), -1);
    //std::fill_n(&distances, graph.GetNodeCount(), std::numeric_limits<int>::max());

    //std::priority_queue<Edge>();
}

