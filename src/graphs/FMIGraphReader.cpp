//
// Created by Jost on 22/04/2024.
//

#include "FMIGraphReader.h"

#include <chrono>
#include <fstream>
#include <iostream>

BasicGraph FMIGraphReader::read(std::string &filePath) {
    // Implementation follows: https://github.com/fmi-alg/OsmGraphCreator/blob/master/readers/fmitextreader.cpp
    // Details from FmiTextGraphWriter in: https://github.com/fmi-alg/OsmGraphCreator/blob/master/creator/GraphWriter.cpp
    std::ifstream fileReadStream;
    fileReadStream.open(filePath);
    if (!fileReadStream.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    // header section - metadata followed by an empty line
    std::string line;
    // skipping metadata
    while (std::getline(fileReadStream, line)) {
        // header section is terminated by an empty line
        if (line.empty()) break;
    }

    int nodeCount;
    fileReadStream >> nodeCount;

    int edgeCount;
    fileReadStream >> edgeCount;

    std::getline(fileReadStream, line); // get remaining new line symbol
    // TODO: check int limits

    std::unique_ptr<Location[]> nodeLocations(new Location[nodeCount]);
    std::unique_ptr<int[]> edgesLookupIndices(new int[nodeCount + 1]); // +1 dummy entry for simplified algorithm
    std::unique_ptr<Edge[]> edges(new Edge[edgeCount]);

    std::cout << "Loading graph with " << nodeCount << " nodes and " << edgeCount << " edges.." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    // node section - all the nodes line by line
    std::cout << "Loading nodes.." << std::endl;
    {
        int nodeId, osmId;
        double lat, lon;
        // assumes lines are sorted by nodeId
        for (int i = 0; i < nodeCount; ++i) {
            //TODO: add eof check
            std::getline(fileReadStream, line);
            std::stringstream lineStream(line);
            lineStream >> nodeId >> osmId >> lat >> lon;
            nodeLocations[i] = {lat, lon};
        };
    }

    //edge section - all the edges line by line
    std::cout << "Loading edges.." << std::endl;
    {
        int lastNodeIndex = -1;
        int curEdgeIndex = 0;

        int source, target, weight;
        // assumes edges are sorted by source node
        for (int i = 0; i < edgeCount; ++i) {
            //TODO: add eof check
            std::getline(fileReadStream, line);
            std::stringstream lineStream(line);
            lineStream >> source >> target >> weight;

            if (lastNodeIndex != source) {
                for (int j = lastNodeIndex + 1; j <= source; j++) {
                    edgesLookupIndices[j] = curEdgeIndex;
                }
                lastNodeIndex = source;
            }

            edges[i] = {target, weight};
            curEdgeIndex++;
        }
        // fill all entries without edges at the back (including the dummy entry) with the edgeCount for simplified
        // retrieval algorithm
        for (int j = lastNodeIndex + 1; j <= nodeCount; j++) {
            edgesLookupIndices[j] = edgeCount;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto loadTimeS = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    std::cout << "Loaded graph in " << loadTimeS.count() << "s" << std::endl;

    return {nodeCount, edgeCount, std::move(nodeLocations), std::move(edgesLookupIndices), std::move(edges)};
}
