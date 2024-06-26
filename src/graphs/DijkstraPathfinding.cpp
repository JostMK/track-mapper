//
// Created by Jost on 19/04/2024.
//

#include "DijkstraPathfinding.h"

#include <limits>
#include <queue>
#include <algorithm>
#include <memory>

DijkstraPathfinding::DijkstraPathfinding(const IGraph &graph) : graph(graph) {
}

Path DijkstraPathfinding::CalculatePath(const int startNodeIndex, const int targetNodeIndex) const {
    auto predecessors = std::make_unique<int[]>(graph.GetNodeCount());
    auto distances = std::make_unique<int[]>(graph.GetNodeCount());

    std::fill_n(predecessors.get(), graph.GetNodeCount(), -1);
    std::fill_n(distances.get(), graph.GetNodeCount(), std::numeric_limits<int>::max());

    std::priority_queue<PriorityQueueEntry, std::vector<PriorityQueueEntry>, std::greater<> > queue;

    distances[startNodeIndex] = 0;
    queue.emplace(startNodeIndex, 0);

    // -- dijkstra algorithm --

    while (!queue.empty()) {
        auto [curNodeIndex, curDistance] = queue.top();
        queue.pop();

        if (distances[curNodeIndex] < curDistance) {
            // popped node is an outdated entry with old distance value
            continue;
        }

        if (curNodeIndex == targetNodeIndex) {
            // reached target node
            break;
        }

        for (auto [edgeTarget, edgeDistance]: graph.GetEdges(curNodeIndex)) {
            int newDistance = curDistance + edgeDistance;
            if (distances[edgeTarget] <= newDistance) {
                // edge is already reachable with shorter path
                continue;
            }

            distances[edgeTarget] = newDistance;
            predecessors[edgeTarget] = curNodeIndex;
            queue.emplace(edgeTarget, newDistance);
        }
    }

    // -- reconstruct path --

    if (predecessors[targetNodeIndex] == -1) {
        // no path was found
        return Path::invalid();
    }

    std::vector<int> path;
    int curNodeIndex = targetNodeIndex;
    while (curNodeIndex != startNodeIndex) {
        path.push_back(curNodeIndex);
        curNodeIndex = predecessors[curNodeIndex];
    }
    path.push_back(startNodeIndex);

    std::ranges::reverse(path);

    return {path, distances[targetNodeIndex]};
}
