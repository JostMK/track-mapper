//
// Created by Jost on 24/05/2024.
//

#include "BasicWebApp.h"

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../mesh/gdal_wrapper.h"
#include "../scene/TrackCreator.h"
#include "TrackData.h"
#include "errors.h"

void TrackWebApp();
bool CreateTrack(TrackData &data);
void OpenWebpage(const std::string &url);

std::unique_ptr<TrackMapper::Web::BasicWebApp> pApp;

void close_gracefully() {
    if (pApp) {
        std::cout << "Closing web server.." << std::endl;
        pApp->Stop();
    }
}
void close_gracefully(const int signal) { close_gracefully(); };


int main() {
    // tries to gracefully clean up if possible
    std::signal(SIGINT, close_gracefully);
    std::signal(SIGTERM, close_gracefully);
    std::atexit(close_gracefully);

    TrackWebApp();
    return 0;
}

void TrackWebApp() {
    try {
        std::cout << "Enter Path to fmi file:" << std::endl;

        std::string filePath;
        std::cin >> filePath;

        pApp = std::make_unique<TrackMapper::Web::BasicWebApp>(filePath);
        TrackData data;

        pApp->Start(data);
        OpenWebpage("http://localhost:18080/static/index.html");

        bool success = false;
        while (!success) {
            std::cout << "Waiting for input from web app.." << std::endl;
            while (!data.IsPopulated()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            std::cout << "Received data.. Creating Track.." << std::endl;
            success = CreateTrack(data);

            if (!success) {
                // resets track data to be set again by web app
                data.rasterFiles.clear();
                data.paths.clear();
                data.projRef = TrackMapper::Raster::ProjectionWrapper();
                // TODO: possible deadlock
                // -> if website sends new create_track request between SetError in CreateTrack and this line
                data.UnSetPopulated();
            }
        }

        std::cout << "Finished Track!" << std::endl;

        // waits for user input before closing console app
        std::cout << "Press ENTER to close process" << std::endl;
        std::cin.ignore();
        std::string await;
        std::getline(std::cin, await);
        std::cout << await; // so variable does not get removed by optimizer

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "An unknown error happend :(" << std::endl;
    }
}

bool CreateTrack(TrackData &data) {
    TrackMapper::Scene::TrackCreator creator(data.name);

    // validate data
    if (data.rasterFiles.empty()) {
        const auto error = ERROR_NO_RASTER;
        std::cout << error << std::endl;
        data.SetError(error);
        return false;
    }

    if (data.paths.empty()) {
        const auto error = ERROR_NO_PATH;
        std::cout << error << std::endl;
        data.SetError(error);
        return false;
    }

    if (data.outputPath.empty()) {
        const auto error = ERROR_NO_OUT_LOC;
        std::cout << error << std::endl;
        data.SetError(error);
        return false;
    }
    const std::filesystem::path outDir(data.outputPath);
    std::error_code ec;
    if (!std::filesystem::is_directory(outDir, ec)) {
        const auto error = ERROR_OUT_NOT_DIR;
        std::cout << error << std::endl;
        data.SetError(error);
        return false;
    }
    if (ec) {
        auto msg = ec.message();
        const auto error = std::vformat(ERROR_INVALID_OUT_LOC, std::make_format_args(msg));
        std::cout << error << std::endl;
        data.SetError(error);
        return false;
    }

    // setup proj ref
    if (data.projRef.Get().empty()) {
        // no projection ref provided: try reading one from the raster
        const TrackMapper::Raster::GDALDatasetWrapper dataset(data.rasterFiles[0]);
        data.projRef = dataset.GetProjectionRef();

        if (!data.projRef.IsValid()) {
            const auto error = ERROR_MISSING_PROJ;
            std::cout << error << std::endl;
            data.SetError(error);
            return false;
        }
    } else {
        if (!data.projRef.IsValid()) {
            auto wkt = data.projRef.Get();
            const auto error = std::vformat(ERROR_INVALID_PROJ, std::make_format_args(wkt));
            std::cout << error << std::endl;
            data.SetError(error);
            return false;
        }
    }

    // creating tiles
    for (int i = 0; i < data.rasterFiles.size(); ++i) {
        const auto progress = std::format("Task 1/4: Creating tile {}/{}", i + 1, data.rasterFiles.size());
        std::cout << progress << std::endl;
        data.SetProgress(progress);
        creator.AddRaster(data.rasterFiles[i]);
    }

    // creating roads
    for (int i = 0; i < data.paths.size(); ++i) {
        const auto progress = std::format("Task 2/4: Creating path {}/{}", i + 1, data.paths.size());
        std::cout << progress << std::endl;
        data.SetProgress(progress);
        creator.AddRoad(data.paths[i], data.projRef);
    }

    // creating spawn
    {
        using Point = TrackMapper::Raster::Point;

        const auto progress = "Task 3/4: Setting spawn points";
        std::cout << progress << std::endl;
        data.SetProgress(progress);

        const auto path = data.paths[0];
        // Note: path[0] will not be added to path using CatmullRom interpolation so path[1] is the edge of the road
        // to have a little bit of safe margin choose path[3] as spawn point
        creator.AddSpawn(path[3], path[4], data.projRef);
    }

    // exporting track
    {
        const auto progress = "Task 4/4: Writing track to disk";
        std::cout << progress << std::endl;
        data.SetProgress(progress);

        creator.Export(data.outputPath);

        data.SetFinished();
    }

    return true;
}

void OpenWebpage(const std::string &url) {
    // following https://stackoverflow.com/questions/17347950/how-do-i-open-a-url-from-c and
    // https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
    // windows: use 'start' in cmd to open url
    const std::string cmd = "start ";
#elif __APPLE__
    // macOS: use 'open' in shell to open url
    const std::string cmd = "open ";
#elif __linux__ || __unix__
    // linux: use 'xdg-open' in shell to open url
    const std::string cmd = "xdg-open ";
#endif // no plattform detected will create an compiler error here since cmd will not be defined

    const auto fullCmd = cmd + url;
    std::cout << "Executing: " << fullCmd << std::endl;
    system(fullCmd.c_str());
}
