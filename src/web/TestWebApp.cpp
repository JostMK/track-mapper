//
// Created by Jost on 02/05/2024.
//

#include "TestWebApp.h"
#include "../graph/DijkstraPathfinding.h"
#include "../graph/SimpleWorldGrid.h"
#include "../mesh/gdal_wrapper.h"
#include "../mesh/raster_reader.h"
#include "crow.h"

#include "error_codes.h"

namespace TrackMapper::Web {

    std::string base64_decode(const std::string &in);

    void TestWebApp::Start(const BasicGraph &graph) {
        const DijkstraPathfinding pathfinding(graph);
        const SimpleWorldGrid grid(graph, 0.01);

        crow::SimpleApp app;
        // app.loglevel(crow::LogLevel::Debug);

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
            for (const auto nodeId: nodeIds) {
                auto [latitude, longitude] = graph.GetLocation(nodeId);
                crow::json::wvalue node;
                node["nodeId"] = nodeId;
                node["lat"] = latitude;
                node["lon"] = longitude;

                path.push_back(node);
            }

            crow::json::wvalue x;
            x["distance"] = distance;
            x["nodes"] = std::move(path);
            return x;
        });

        // get extends rect of a raster
        // REQ: base64 encoded json obj containing filepath to raster and optionally custom proj ref
        // RES: 4 points representing the corners of the raster rect
        CROW_ROUTE(app, "/api/get_raster_extend/<string>")
        ([](const std::string &base64JsonObj) {
            auto rasterJson = crow::json::load(base64_decode(base64JsonObj));

            std::string rasterFilePath = rasterJson["filePath"].s();
            const Raster::GDALDatasetWrapper dataset(rasterFilePath);

            if (!dataset.isValid()) {
                crow::json::wvalue x;
                x["error"] = ERROR_INVALID_FILE + " Failed to open file: " + rasterFilePath;
                return x;
            }

            OGRSpatialReference srcProjRef(dataset.GetProjectionRef().exportToWkt().c_str());
            if (srcProjRef.Validate() != OGRERR_NONE) {
                // check if custom proj ref was provided
                if (!rasterJson.has("projRef")) {
                    crow::json::wvalue x;
                    x["error"] = ERROR_MISSING_PROJ + " File misses projection reference, please manually specify it!";
                    return x;
                }

                // get custom proj ref
                std::string customProjRef = rasterJson["projRef"].s();
                srcProjRef = OGRSpatialReference(customProjRef.c_str());
                // validate custom proj ref
                if (srcProjRef.Validate() != OGRERR_NONE) {
                    crow::json::wvalue x;
                    x["error"] = ERROR_INVALID_PROJ +
                                 " Provided projection reference does not discribe a valide projection:\n\n" +
                                 customProjRef;
                    return x;
                }
            }

            auto extends = Raster::getDatasetExtends(dataset);
            if (auto success = Raster::reprojectPoints(extends, srcProjRef, Raster::osmPointsProjRef); !success) {
                crow::json::wvalue x;
                x["error"] = ERROR_FAILED_PROJ + " Failed to project points to WGS84!";
                return x;
            }

            std::vector<crow::json::wvalue> corners;
            for (const auto [lat, lon]: extends) {
                crow::json::wvalue node;
                node["lat"] = lat;
                node["lon"] = lon;

                corners.push_back(node);
            }

            crow::json::wvalue x;
            x["corners"] = std::move(corners);
            return x;
        });

        app.port(18080).run();
    }

    // copied from https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    std::string base64_decode(const std::string &in) {
        std::string out;

        std::vector T(256, -1);
        for (int i = 0; i < 64; i++)
            T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

        int val = 0, valb = -8;
        for (const unsigned char c: in) {
            if (T[c] == -1)
                break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(static_cast<char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }
} // namespace TrackMapper::Web
