//
// Created by Jost on 16/04/2024.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>

struct Location {
    long latitude;
    long longitude;
};

struct Edge {
    int adjacentNodeIndex;
    int distance;
};

class Graph {
public:
    virtual std::vector<Edge> GetEdges(int nodeIndex) = 0;
    virtual Location GetLocation(int nodeIndex) = 0;
};

#endif //GRAPH_H
