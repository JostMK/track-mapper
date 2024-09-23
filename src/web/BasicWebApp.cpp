//
// Created by Jost on 02/05/2024.
//

#include "BasicWebApp.h"

#include "crow.h"

#include "../graph/DijkstraPathfinding.h"
#include "../graph/FMIGraphReader.h"
#include "../graph/SimpleWorldGrid.h"
#include "../mesh/gdal_wrapper.h"
#include "../mesh/raster_reader.h"

#include "errors.h"

namespace TrackMapper::Web {
    struct BasicWebApp::impl {
        BasicGraph mGraph;
        SimpleWorldGrid mGrid;
        DijkstraPathfinding mPathfinding;

        crow::SimpleApp app;
        std::future<void> runner; // needed for async execution of webserver

        explicit BasicWebApp::impl(const std::string &filePath) try :
            mGraph{FMIGraphReader::read(filePath)}, mGrid{mGraph, 0.01}, mPathfinding{mGraph} {
        } catch (...) {
        }
    };

    std::string base64_decode(const std::string &in);

    BasicWebApp::BasicWebApp(const std::string &filePath) try : pImpl{std::make_unique<impl>(filePath)} {
    } catch (...) {
    }
    BasicWebApp::~BasicWebApp() = default; // needed for compile pImpl ideom

    void BasicWebApp::Start(TrackData &trackData) const {
#ifdef NDEBUG
        pImpl->app.loglevel(crow::LogLevel::Error);
#else
        pImpl->app.loglevel(crow::LogLevel::Info);
#endif

        // get closest node to mouse click endpoint
        // REQ: latitude and longitude as double/double
        // RES: node id as json string
        CROW_ROUTE(pImpl->app, "/api/get_node/<double>/<double>")
        ([&grid = pImpl->mGrid](const double lat, const double lon) {
            const int closestNode = grid.GetClosestNode({lat, lon});

            crow::json::wvalue x;
            x["nodeId"] = closestNode;
            return x;
        });

        // get closest node to mouse click endpoint
        // REQ: latitude and longitude as double/double
        // RES: node id as json string
        CROW_ROUTE(pImpl->app, "/api/get_location/<int>")
        ([&mGraph = pImpl->mGraph](const int node_id) {
            auto [latitude, longitude] = mGraph.GetLocation(node_id);

            crow::json::wvalue x;
            x["lat"] = latitude;
            x["lon"] = longitude;
            return x;
        });

        // get the shortest path between two nodes
        // REQ: start and target node id as int/int
        // RES: shortest path as json string
        CROW_ROUTE(pImpl->app, "/api/get_path/<int>/<int>")
        ([&pathfinding = pImpl->mPathfinding, &mGraph = pImpl->mGraph](const int startNodeIndex,
                                                                       const int targetNodeIndex) {
            auto [nodeIds, distance] = pathfinding.CalculatePath(startNodeIndex, targetNodeIndex);

            std::vector<crow::json::wvalue> path;
            path.reserve(nodeIds.size());
            for (const auto nodeId: nodeIds) {
                auto [latitude, longitude] = mGraph.GetLocation(nodeId);
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
        CROW_ROUTE(pImpl->app, "/api/get_raster_extend/<string>")
        ([](const std::string &base64JsonObj) {
            auto rasterJson = crow::json::load(base64_decode(base64JsonObj));

            std::string rasterFilePath = rasterJson["filePath"].s();
            const Raster::GDALDatasetWrapper dataset(rasterFilePath);

            if (!dataset.IsValid()) {
                crow::json::wvalue x;
                x["error"] = std::vformat(ERROR_INVALID_FILE, std::make_format_args(rasterFilePath));
                return x;
            }

            auto srcProjRef = dataset.GetProjectionRef();
            if (!srcProjRef.IsValid()) {
                // check if custom proj ref was provided
                if (!rasterJson.has("projRef")) {
                    crow::json::wvalue x;
                    x["error"] = ERROR_MISSING_PROJ;
                    return x;
                }

                // get custom proj ref
                std::string customProjRef = rasterJson["projRef"].s();
                srcProjRef = Raster::ProjectionWrapper(customProjRef);
                // validate custom proj ref
                if (!srcProjRef.IsValid()) {
                    crow::json::wvalue x;
                    x["error"] = std::vformat(ERROR_INVALID_PROJ, std::make_format_args(customProjRef));
                    return x;
                }
            }

            auto extends = Raster::getDatasetExtends(dataset);
            if (auto success = Raster::reprojectPoints(extends, srcProjRef, Raster::osmPointsProjRef); !success) {
                crow::json::wvalue x;
                x["error"] = ERROR_FAILED_PROJ;
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

        // starts track creation
        // REQ: base64 encoded json obj containing data for track creation
        // RES: error msg if error happens
        CROW_ROUTE(pImpl->app, "/api/create_track/<string>")
        ([&trackData, &mGraph = pImpl->mGraph](const std::string &base64JsonObj) {
            const auto trackJson = crow::json::load(base64_decode(base64JsonObj));

            const std::string name = trackJson["name"].s();
            // if lo() misses const modifier please update crow past commit
            // https://github.com/CrowCpp/Crow/commit/a9e7b7321b0f7ef082cf509b762755136683beaf
            // or manually modify header
            const auto rastersJson = trackJson["rasters"].lo();
            const auto pathsJson = trackJson["paths"].lo();

            trackData.name = name;

            // if wkt non empty validate and use it
            if (const std::string wkt = trackJson["wkt"].s(); !wkt.empty()) {
                Raster::ProjectionWrapper proj(wkt);
                if (!proj.IsValid()) {
                    crow::json::wvalue x;
                    x["error"] = std::vformat(ERROR_INVALID_PROJ, std::make_format_args(wkt));
                    return x;
                }
                trackData.projRef = proj;
            }

            trackData.rasterFiles.reserve(rastersJson.size());
            for (const auto &rasterPath: rastersJson) {
                trackData.rasterFiles.push_back(rasterPath.s());
            }

            trackData.paths.resize(pathsJson.size());
            for (int i = 0; i < pathsJson.size(); ++i) {
                const auto pathJson = pathsJson[i].lo();
                trackData.paths[i].reserve(pathJson.size());
                for (const auto &node: pathJson) {
                    const auto nodeId = static_cast<int>(node.i()); // node id should not be bigger then int
                    const auto [lat, lng] = mGraph.GetLocation(nodeId);
                    trackData.paths[i].emplace_back(lat, lng);
                }
            }

            trackData.SetPopulated();
            trackData.SetProgress("Submitting Track Data");

            crow::json::wvalue x;
            x["status"] = "ok";
            return x;
        });

        // gets progress msg of track creation progress
        // RES: string containing progress msg
        CROW_ROUTE(pImpl->app, "/api/get_progress")
        ([&trackData]() {
            crow::json::wvalue x;
            x["progress"] = trackData.GetProgress();
            x["finished"] = trackData.IsFinished();
            if (const auto error = trackData.GetError(); !error.empty())
                x["error"] = error;

            return x;
        });

        std::cout << "Starting web app.." << std::endl;
        pImpl->runner = pImpl->app.port(18080).run_async();
    }

    void createTrack() {}

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
