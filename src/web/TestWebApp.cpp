//
// Created by Jost on 02/05/2024.
//

#include "TestWebApp.h"
#include "crow.h"
#include "../graph/DijkstraPathfinding.h"
#include "../graph/SimpleWorldGrid.h"


void TestWebApp::Start(const BasicGraph &graph) {
    const DijkstraPathfinding pathfinding(graph);
    const SimpleWorldGrid grid(graph, 0.01);

    crow::SimpleApp app;
    //app.loglevel(crow::LogLevel::Debug);

    // get closest node to mouse click endpoint
    // REQ: latitude and longitude as double/double
    // RES: node id as json string
    CROW_ROUTE(app, "/api/get_node/<double>/<double>")
    ([&grid](const double lat, const double lon) {
        const int closestNode = grid.GetClosestNode({lat, lon});

        crow::json::wvalue x;
        x["nodeId"] = closestNode;
        return x;
    });

    // get closest node to mouse click endpoint
    // REQ: latitude and longitude as double/double
    // RES: node id as json string
    CROW_ROUTE(app, "/api/get_location/<int>")
    ([&graph](const int node_id) {
        auto [latitude, longitude] = graph.GetLocation(node_id);

        crow::json::wvalue x;
        x["lat"] = latitude;
        x["lon"] = longitude;
        return x;
    });

    // get the shortest path between two nodes
    // REQ: start and target node id as int/int
    // RES: shortest path as json string
    CROW_ROUTE(app, "/api/get_path/<int>/<int>")
    ([&pathfinding, &graph](const int startNodeIndex, const int targetNodeIndex) {
        auto [nodeIds, distance] = pathfinding.CalculatePath(startNodeIndex, targetNodeIndex);

        std::vector<crow::json::wvalue> path;
        path.reserve(nodeIds.size());
        for (auto nodeId : nodeIds){
            auto location = graph.GetLocation(nodeId);
            crow::json::wvalue node;
            node["nodeId"] = nodeId;
            node["lat"] = location.latitude;
            node["lon"] = location.longitude;

            path.push_back(node);
        }

        crow::json::wvalue x;
        x["distance"] = distance;
        x["nodes"] = std::move(path);
        return x;
    });

    app.port(18080).run();
}
