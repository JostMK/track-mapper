#include <iostream>
#include "src/graphs/FMIGraphReader.h"


int main()
{
    std::cout << "Enter Path to fmi file:" << std::endl;

    std::string filePath;
    std::cin>>filePath;
    BasicGraph graph = FMIGraphReader::read(filePath);
    /**
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        auto [latitude, longitude] = graph.GetLocation(i);
        std::cout << "Location " << i << ": " << latitude << " : " << longitude << std::endl;

        if(auto e = graph.GetEdges(i); !e.empty()) {
            for (auto [adjacentNodeIndex, distance]: e) {
                std::cout << " > Edge " << i << " -> " << adjacentNodeIndex << " : " << distance << std::endl;
            }
        }
    }
    **/

    int nodeIndex;
    while(true) {
        std::cout << "Enter node id or enter -1 to exit:" << std::endl;
        std::cin >> nodeIndex;

        if(nodeIndex < 0)
            break;

        auto [lat, lon] = graph.GetLocation(nodeIndex);
        std::cout << "Node " << nodeIndex << ": " << lat << " ; " << lon << std::endl;
        if(auto e = graph.GetEdges(nodeIndex); !e.empty()) {
            for (auto [adjacentNodeIndex, distance]: e) {
                std::cout << " > Edge " << nodeIndex << " -> " << adjacentNodeIndex << " : " << distance << std::endl;
            }
        }
    }

    return 0;
}
