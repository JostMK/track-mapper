//
// Created by Jost on 24/05/2024.
//

#include "BasicWebApp.h"

#include <iostream>
#include "../mesh/gdal_wrapper.h"
#include "../scene/TrackCreator.h"
#include "TrackData.h"

void TrackWebApp();
void CreateTrack(TrackData &data);
void OpenWebpage(const std::string &url);

int main() {
    TrackWebApp();
    return 0;
}

void TrackWebApp() {
    try {
        std::cout << "Enter Path to fmi file:" << std::endl;

        std::string filePath;
        std::cin >> filePath;

        TrackMapper::Web::BasicWebApp app(filePath);
        TrackData data;

        app.Start(data);
        OpenWebpage("http://localhost:18080/static/index.html");

        std::cout << "Waiting for input from web app.." << std::endl;
        while (!data.IsPopulated()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Received data.. Creating Track.." << std::endl;
        CreateTrack(data);

        std::cout << "Finished Track!" << std::endl;
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "An unknown error happend :(" << std::endl;
    }
}

void CreateTrack(TrackData &data) {
    TrackMapper::Scene::TrackCreator creator(data.name);

    for (int i = 0; i < data.rasterFiles.size(); ++i) {
        const auto progress = std::format("Task 1/4: Creating tile {}/{}", i, data.rasterFiles.size());
        std::cout << progress << std::endl;
        data.SetProgress(progress);
        creator.AddRaster(data.rasterFiles[i]);
    }

    if (!data.projRef.IsValid()) {
        // TODO: fail if provided projRef is invalid
    } else {
        const TrackMapper::Raster::GDALDatasetWrapper dataset(data.rasterFiles[0]);
        data.projRef = dataset.GetProjectionRef();

        if (!data.projRef.IsValid()) {
            // TODO: fail if raster does not contain projRef and a custom one was also not supplied
        }
    }

    for (int i = 0; i < data.paths.size(); ++i) {
        const auto progress = std::format("Task 2/4: Creating path {}/{}", i, data.paths.size());
        std::cout << progress << std::endl;
        data.SetProgress(progress);
        creator.AddPath(data.paths[i], data.projRef);
    }

    {
        using Point = TrackMapper::Raster::Point;

        const auto progress = "Task 3/4: Setting spawn points";
        std::cout << progress << std::endl;
        data.SetProgress(progress);

        const auto path = data.paths[0];
        // path[0] will not be added to path using CatmullRom interpolation so path[1] is the edge of the road
        // to have a little bit of safe margin choose path[3] as spawn point
        const auto pit = Point{path[3].lat, 0, path[3].lng}; // height will be set in TrackCreator
        const auto dir = Point{path[4].lat, 0, path[4].lng} - pit;
        creator.AddSpawn(pit, dir);
    }

    {
        const auto progress = "Task 4/4: Writing track to disk";
        std::cout << progress << std::endl;
        data.SetProgress(progress);

        // TODO: make this a field from the website
        std::cout << "Enter path for exported track folder:" << std::endl;
        std::string outFilePath;
        std::cin >> outFilePath;
        creator.Export(outFilePath);
    }
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
