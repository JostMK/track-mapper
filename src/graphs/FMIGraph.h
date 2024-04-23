//
// Created by Jost on 16/04/2024.
//

#ifndef FMIGRAPH_H
#define FMIGRAPH_H

#include "IGraph.h"

class FMIGraph final : public IGraph {
public:
    explicit FMIGraph(const std::string& filePath);

    int GetNodeCount() const override;
    std::vector<Edge> GetEdges(int nodeIndex) override;
    Location GetLocation(int nodeIndex) const override;

private:
    std::vector<Location> nodeLocations;
    std::vector<int> edgeListLookup;
    std::vector<Edge> edges;
};



#endif //FMIGRAPH_H
