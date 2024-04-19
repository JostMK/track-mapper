//
// Created by Jost on 16/04/2024.
//

#include <fstream>
#include <iostream>
#include <cstdint>
#include <sstream>
#include "FMIGraph.h"

#include <chrono>

//TODO: clean up and make more robust
FMIGraph::FMIGraph(const std::string& filePath) {
    // Implementation follows: https://github.com/fmi-alg/OsmGraphCreator/blob/master/readers/fmitextreader.cpp
    // Details from FmiTextGraphWriter in: https://github.com/fmi-alg/OsmGraphCreator/blob/master/creator/GraphWriter.cpp
    std::ifstream fileReadStream;
    fileReadStream.open(filePath);
    if (!fileReadStream.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    // header section - 7 lines of metadata:
    std::string line;
    // skipping unrelevant data
    while(std::getline(fileReadStream, line)) {
        if(line.empty()) // header section is seperated by newline at the end
            break;
    }

    uint32_t nodeCount;
    fileReadStream >> nodeCount;

    uint32_t edgeCount;
    fileReadStream >> edgeCount;

    std::getline(fileReadStream, line); // get remaining new line symbol
    // TODO: check int limits

    std::cout << "Loading graph with " << nodeCount << " nodes and " << edgeCount << " edges.." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    // node section - all the nodes line by line
    std::cout << "Loading nodes.." << std::endl;
    {
        nodeLocations.reserve(nodeCount);

        int32_t nodeId;
        int64_t osmId;
        double lat, lon;
        int32_t elev;
        // assumes nodeIds linearly counts up from 0 to nodeCount-1
        for (int i = 0; i < nodeCount; ++i) {
            //TODO: add eof check
            std::getline(fileReadStream, line);
            std::stringstream lineStream(line);
            lineStream >> nodeId >> osmId >> lat >> lon >> elev;
            nodeLocations.emplace_back(lat, lon);
        }
    }

    //edge section - all the edges line by line
    std::cout << "Loading edges.." << std::endl;
    {
        edgeListLookup.reserve(nodeCount + 1); // one dummy element at the end for simpler retrival algorithm
        edges.reserve(edgeCount);
        int lastIndex = -1;
        int curEdgeIndex = 0;

        int32_t source;
        int32_t target;
        int32_t weight;
        int32_t type;
        // assumes edges are sorted by source node
        for (int i = 0; i < edgeCount; ++i) {
            //TODO: add eof check
            std::getline(fileReadStream, line);
            std::stringstream lineStream(line);
            lineStream >> source >> target >> weight >> type;

            if(lastIndex != source) {
                for (int j = lastIndex + 1; j <= source; j++) {
                    edgeListLookup.push_back(curEdgeIndex);
                }
                lastIndex = source;
            }

            edges.emplace_back(target, weight);
            curEdgeIndex++;
        }
        for (int j = lastIndex + 1; j <= nodeCount; j++) {
            edgeListLookup.push_back(static_cast<int>(edgeCount));
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto loadTimeS = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    std::cout << "Loaded graph in " << loadTimeS.count() << "s" << std::endl;
}

int FMIGraph::GetNodeCount() {
    //TODO: dont cast idk ¯\_(ツ)_/¯
    return static_cast<int>(nodeLocations.size());
}

std::vector<Edge> FMIGraph::GetEdges(const int nodeIndex) {
    const int startIndex = edgeListLookup[nodeIndex];
    const int nextNodeStartIndex = edgeListLookup[nodeIndex+1];

    return {&edges[startIndex], &edges[nextNodeStartIndex]};
}

Location FMIGraph::GetLocation(const int nodeIndex) {
    return nodeLocations[nodeIndex];
}
