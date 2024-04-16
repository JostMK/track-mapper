//
// Created by Jost on 16/04/2024.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>

struct Edge {
    int adjacentNodeIndex;
    int distance;
};

class Graph {
public:
    Graph(std::string inputFilename);
    virtual ~Graph() = 0;
    virtual void Save(std::string outputFilename) = 0;

    virtual std::vector<Edge> GetEdges(int nodeIndex);

private:
    int nodeCount;
    std::vector<int> edgesLookup; // lookup into nodesEdges to where the edges of a node start
    std::vector<Edge> nodesEdges; // stores the adjacency list flatted out

    virtual void Load(std::string inputFilename) = 0;
};



#endif //GRAPH_H
