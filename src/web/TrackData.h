//
// Created by Jost on 22/09/2024.
//

#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <mutex>
#include <string>
#include <vector>

#include "../mesh/raster_reader.h"

struct TrackData {
    using Path = std::vector<TrackMapper::Raster::OSMPoint>;

    std::string name;
    std::vector<std::string> rasterFiles;
    std::vector<Path> paths;
    TrackMapper::Raster::ProjectionWrapper projRef;

private:
    std::string progress;
    std::string error;
    bool isPopulated = false;
    bool isFinished = false;

    std::mutex mutex;

public:
    void SetPopulated() {
        const std::lock_guard lock(mutex);
        isPopulated = true;
    }
    void UnSetPopulated() {
        const std::lock_guard lock(mutex);
        isPopulated = false;
    }

    [[nodiscard]] bool IsPopulated() {
        const std::lock_guard lock(mutex);
        return isPopulated;
    }

    void SetFinished() {
        const std::lock_guard lock(mutex);
        isFinished = true;
    }

    [[nodiscard]] bool IsFinished() {
        const std::lock_guard lock(mutex);
        return isFinished;
    }

    void SetProgress(const std::string &progressText) {
        const std::lock_guard lock(mutex);
        progress = progressText;
    }

    [[nodiscard]] std::string GetProgress() {
        const std::lock_guard lock(mutex);
        return progress; // copies string
    }

    void SetError(const std::string &errorText) {
        const std::lock_guard lock(mutex);
        error = errorText;
    }

    [[nodiscard]] std::string GetError() {
        const std::lock_guard lock(mutex);
        return error; // copies string
    }
};

#endif // TRACKDATA_H
