
#include <chrono>
#include <iostream>

#include "BasicGraph.h"
#include "DijkstraPathfinding.h"
#include "FMIGraphreader.h"

void PrintGraph(const BasicGraph &graph);

void QueryGraph(const BasicGraph &graph);

void QueryShortestPath(const BasicGraph &graph);

int main() {
    std::cout << "Enter Path to fmi file:" << std::endl;

    std::string filePath;
    std::cin >> filePath;
    const BasicGraph graph = FMIGraphReader::read(filePath);

    bool run = true;
    while (run) {
        std::cout << "Options: Print Graph (g); Query Graph Node (n); Query Shortest Path (p); Quit (q)" << std::endl;
        std::string option;
        std::cin >> option;

        switch (option[0]) {
            case 'q': run = false;
                break;
            case 'g': PrintGraph(graph);
                break;
            case 'n': QueryGraph(graph);
                break;
            case 'p': QueryShortestPath(graph);
                break;
            default: std::cout << "Use one of the options: " << std::endl;
                break;
        }
    }
}

void PrintGraph(const BasicGraph &graph) {
    for (int i = 0; i < graph.GetNodeCount(); ++i) {
        auto [latitude, longitude] = graph.GetLocation(i);
        std::cout << "Location " << i << ": " << latitude << " : " << longitude << std::endl;

        if (auto e = graph.GetEdges(i); !e.empty()) {
            for (auto [adjacentNodeIndex, distance]: e) {
                std::cout << " > Edge " << i << " -> " << adjacentNodeIndex << " : " << distance << std::endl;
            }
        }
    }
}

void QueryGraph(const BasicGraph &graph) {
    while (true) {
        std::cout << "Enter node id or enter -1 to exit:" << std::endl;
        int nodeIndex;
        std::cin >> nodeIndex;

        if (nodeIndex < 0)
            break;

        auto [lat, lon] = graph.GetLocation(nodeIndex);
        std::cout << "Node " << nodeIndex << ": " << lat << " ; " << lon << std::endl;
        if (auto e = graph.GetEdges(nodeIndex); !e.empty()) {
            for (auto [adjacentNodeIndex, distance]: e) {
                std::cout << " > Edge " << nodeIndex << " -> " << adjacentNodeIndex << " : " << distance << std::endl;
            }
        }
    }
}

void QueryShortestPath(const BasicGraph &graph) {
    const DijkstraPathfinding dijkstra(graph);

    while (true) {
        std::cout << "Enter start node id or enter -1 to exit:" << std::endl;
        int startNodeIndex;
        std::cin >> startNodeIndex;

        if (startNodeIndex < 0)
            break;

        std::cout << "Enter target node id or enter -1 to exit:" << std::endl;
        int targetNodeIndex;
        std::cin >> targetNodeIndex;

        if (targetNodeIndex < 0)
            break;

        auto startTime = std::chrono::high_resolution_clock::now();

        auto [nodeIds, distance] = dijkstra.CalculatePath(startNodeIndex, targetNodeIndex);

        auto endTime = std::chrono::high_resolution_clock::now();
        auto loadTimeS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Query in " << loadTimeS.count() << "ms" << std::endl;
        std::cout << "Distance: " << distance << std::endl;
    }
}
