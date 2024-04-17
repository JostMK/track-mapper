//
// Created by Jost on 16/04/2024.
//

#ifndef FMIGRAPH_H
#define FMIGRAPH_H

#include "Graph.h"

class FMIGraph: public Graph {
public:
     explicit FMIGraph(const std::string& filePath);

private:
    inline void PopulateAsEmpty();

    std::vector<Location> nodeLocations;
    std::vector<int> edgeListLookup;
    std::vector<Edge> edges;
};



#endif //FMIGRAPH_H
