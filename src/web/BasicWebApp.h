//
// Created by Jost on 02/05/2024.
//

#ifndef TESTWEBAPP_H
#define TESTWEBAPP_H

#include <string>
#include "crow.h"

#include "../graph/BasicGraph.h"
#include "TrackData.h"

namespace TrackMapper::Web {
    class BasicWebApp {
    public:
        explicit BasicWebApp(const std::string &filePath);
        void Start(TrackData &trackData);

    private:
        BasicGraph mGraph;
        crow::SimpleApp app{};
    };
} // namespace TrackMapper::Web

#endif // TESTWEBAPP_H
