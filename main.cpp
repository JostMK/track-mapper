
#include <chrono>
#include <iostream>
#include "src/graphs/FMIGraphReader.h"
#include "src/graphs/DijkstraPathfinding.h"

static void PrintGraph(BasicGraph& graph);
static void QueryGraph(BasicGraph& graph);
static void QueryShortestPath(BasicGraph& graph);

int main()
{
    std::cout << "Enter Path to fmi file:" << std::endl;

    std::string filePath;
    std::cin>>filePath;
    BasicGraph graph = FMIGraphReader::read(filePath);

    QueryShortestPath(graph);

    return 0;
}


static void PrintGraph(BasicGraph& graph){
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        auto [latitude, longitude] = graph.GetLocation(i);
        std::cout << "Location " << i << ": " << latitude << " : " << longitude << std::endl;

        if(auto e = graph.GetEdges(i); !e.empty()) {
            for (auto [adjacentNodeIndex, distance]: e) {
                std::cout << " > Edge " << i << " -> " << adjacentNodeIndex << " : " << distance << std::endl;
            }
        }
    }
}

static void QueryGraph(BasicGraph& graph){
    while(true) {
        std::cout << "Enter node id or enter -1 to exit:" << std::endl;
        int nodeIndex;
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
}

static void QueryShortestPath(BasicGraph& graph){
    DijkstraPathfinding dijkstra(graph);

    while(true) {
        std::cout << "Enter start node id or enter -1 to exit:" << std::endl;
        int startNodeIndex;
        std::cin >> startNodeIndex;

        if(startNodeIndex < 0)
            break;

        std::cout << "Enter target node id or enter -1 to exit:" << std::endl;
        int targetNodeIndex;
        std::cin >> targetNodeIndex;

        if(targetNodeIndex < 0)
            break;

        auto startTime = std::chrono::high_resolution_clock::now();

        auto path = dijkstra.CalculatePath(startNodeIndex, targetNodeIndex);

        auto endTime = std::chrono::high_resolution_clock::now();
        auto loadTimeS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Query in " << loadTimeS.count() << "ms" << std::endl;
        std::cout << "Distance: " << path.distance << std::endl;
    }
}
