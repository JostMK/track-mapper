//
// Created by Jost on 19/04/2024.
//

#ifndef DIJKSTRAPATHFINDING_H
#define DIJKSTRAPATHFINDING_H

#include "IGraph.h"

struct Path {
    std::vector<int> nodeIds;
    int distance;

    static Path invalid() {
        return {std::vector<int>(), -1};
    }
};

struct PriorityQueueEntry {
    int nodeIndex;
    int priority;

    friend bool operator<(const PriorityQueueEntry &left, const PriorityQueueEntry &right) {
        return left.priority < right.priority;
    }

    friend bool operator>(const PriorityQueueEntry &left, const PriorityQueueEntry &right) {
        return left.priority > right.priority;
    }
};

class DijkstraPathfinding {
public:
    explicit DijkstraPathfinding(const IGraph &graph);

    [[nodiscard]] Path CalculatePath(int startNodeIndex, int targetNodeIndex) const;

private:
    const IGraph &graph;
};


#endif //DIJKSTRAPATHFINDING_H
