//
// Created by Jost on 19/04/2024.
//

#include "DijkstraPathfinding.h"

#include <limits>
#include <queue>
#include <algorithm>

DijkstraPathfinding::DijkstraPathfinding(const IGraph &graph) : graph(graph) {
    predecessors = std::make_unique<int[]>(graph.GetNodeCount());
    distances = std::make_unique<int[]>(graph.GetNodeCount());
}

Path DijkstraPathfinding::CalculatePath(int startNodeIndex, int targetNodeIndex) {
    std::fill_n(predecessors.get(), graph.GetNodeCount(), -1);
    std::fill_n(distances.get(), graph.GetNodeCount(), std::numeric_limits<int>::max());

    std::priority_queue<PriorityQueueEntry, std::vector<PriorityQueueEntry>, std::greater<>> queue;

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

        auto edges = graph.GetEdges(curNodeIndex);
        for (auto [edgeTarget, edgeDistance]: edges) {
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

    if(predecessors[targetNodeIndex] == -1){
        // no path was found
        return Path::invalid();
    }

    std::vector<int> path;
    int curNodeIndex = targetNodeIndex;
    while (curNodeIndex != startNodeIndex){
        path.push_back(curNodeIndex);
        curNodeIndex = predecessors[curNodeIndex];
    }
    path.push_back(startNodeIndex);

    std::reverse(path.begin(), path.end());

    return {path, distances[targetNodeIndex]};
}

