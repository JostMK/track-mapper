//
// Created by Jost on 02/05/2024.
//

#include "TestWebApp.h"
#include "crow.h"
#include "../graphs/DijkstraPathfinding.h"


void TestWebApp::Start(const BasicGraph &graph) {
    const DijkstraPathfinding pathfinding(graph);
    const int graphSize = graph.GetNodeCount();

    crow::SimpleApp app;

    // get closest node to mouse click endpoint
    // REQ: latitude and longitude as double/double
    // RES: node id as json string
    CROW_ROUTE(app, "/api/get_node/<double>/<double>")
    ([&graphSize](double lat, double lon) {
        // TODO: Allow to find closest node in graph
        crow::json::wvalue x;
        x["node_id"] = std::rand() % graphSize;
        return x;
    });

    // get shortest path between two nodes
    // REQ: start and target node id as int/int
    // RES: shortest path as json string
    CROW_ROUTE(app, "/api/get_path/<int>/<int>")
    ([&pathfinding](const int startNodeIndex, const int targetNodeIndex) {
        auto [path, distance] = pathfinding.CalculatePath(startNodeIndex, targetNodeIndex);

        crow::json::wvalue x;
        x["distance"] = distance;
        x["path"] = path;
        return x;
    });

    app.port(18080).run();
}
