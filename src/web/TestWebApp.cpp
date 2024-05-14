//
// Created by Jost on 02/05/2024.
//

#include "TestWebApp.h"
#include "crow.h"
#include "../graphs/DijkstraPathfinding.h"
#include "../graphs/SimpleWorldGrid.h"


void TestWebApp::Start(const BasicGraph &graph) {
    const DijkstraPathfinding pathfinding(graph);
    const SimpleWorldGrid grid(graph, 0.01);
    const int graphSize = graph.GetNodeCount();

    crow::SimpleApp app;

    // get closest node to mouse click endpoint
    // REQ: latitude and longitude as double/double
    // RES: node id as json string
    CROW_ROUTE(app, "/api/get_node/<double>/<double>")
    ([&graphSize](const double lat, const double lon) {
        // TODO: Allow to find closest node in graph
        crow::json::wvalue x;
        x["nodeId"] = std::rand() % graphSize;
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

    // get shortest path between two nodes
    // REQ: start and target node id as int/int
    // RES: shortest path as json string
    CROW_ROUTE(app, "/api/get_path/<int>/<int>")
    ([&pathfinding](const int startNodeIndex, const int targetNodeIndex) {
        auto [nodeIds, distance] = pathfinding.CalculatePath(startNodeIndex, targetNodeIndex);

        crow::json::wvalue x;
        x["distance"] = distance;
        x["nodes"] = nodeIds;
        return x;
    });

    //TODO: Remove after testing
    CROW_ROUTE(app, "/api/get_cell/<double>/<double>")
    ([&grid](const double lat, const double lon) {
        const int cellIndex = grid.GetCellIndexForLocation({lat, lon});

        crow::json::wvalue x;
        x["cellIndex"] = cellIndex;
        return x;
    });

    CROW_ROUTE(app, "/api/get_nodes_in_cell/<int>")
    ([&grid](const int cellIndex) {
        const auto nodes = grid.GetNodeIndicesInCell(cellIndex);

        crow::json::wvalue x;
        x["nodes"] = nodes;
        return x;
    });

    app.port(18080).run();
}
