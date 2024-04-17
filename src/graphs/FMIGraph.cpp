//
// Created by Jost on 16/04/2024.
//

#include <fstream>
#include <iostream>
#include <cstdint>
#include "FMIGraph.h"

FMIGraph::FMIGraph(const std::string& filePath) {
    std::ifstream fileReadStream;
    fileReadStream.open(filePath);

    if(!fileReadStream.good()){
        if(fileReadStream.eof()){ // End of File reached
            std::cout << "File is empty" << std::endl;
            PopulateAsEmpty();
            return;
        }
        if(fileReadStream.bad()){
            std::cout << "Read error on i/o operation" << std::endl;
            PopulateAsEmpty();
            return;
        }
        if(fileReadStream.fail()){
            std::cout << "Logical error on i/o operation" << std::endl;
            PopulateAsEmpty();
            return;
        }
    }

    // read in file
    std::string line;
    // - 6 lines of metadata
    /**
     * Id : <unsigned integer>
Timestamp : <UNIX timestamp>
Type: <type of the graph>
Revision: <unsigned integer>
<number of nodes as unsigned integer>
<number of edges as unsigned integer>
fmitext
     */
    std::getline(fileReadStream, line); // id
    std::getline(fileReadStream, line); // timestamp
    std::getline(fileReadStream, line); // type
    std::getline(fileReadStream, line); // revision

    uint32_t nodeCount;
    fileReadStream>>nodeCount;
    nodeLocations.reserve(nodeCount);
    edgeListLookup.reserve(nodeCount);

    uint32_t edgeCount;
    fileReadStream>>edgeCount;
    edges.reserve(edgeCount);

    // - all the nodes line by line
    // - all the edges line by line
    // - construct graph
}

void FMIGraph::PopulateAsEmpty() {
    // nothing to do
}
