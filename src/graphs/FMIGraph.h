//
// Created by Jost on 16/04/2024.
//

#ifndef FMIGRAPH_H
#define FMIGRAPH_H

#include "Graph.h"

class FMIGraph final : public Graph {
public:
    explicit FMIGraph(const std::string& filePath);

    int GetNodeCount() override;
    std::vector<Edge> GetEdges(int nodeIndex) override;
    Location GetLocation(int nodeIndex) override;

private:
    std::vector<Location> nodeLocations;
    std::vector<int> edgeListLookup;
    std::vector<Edge> edges;
};



#endif //FMIGRAPH_H
