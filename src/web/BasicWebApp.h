//
// Created by Jost on 02/05/2024.
//

#ifndef TESTWEBAPP_H
#define TESTWEBAPP_H

#include <string>
#include "../graph/BasicGraph.h"

namespace TrackMapper::Web {
    class BasicWebApp {
    public:
        explicit BasicWebApp(const std::string &filePath);
        void Start();

    private:
        BasicGraph mGraph;
        std::string mProgressText;
    };
} // namespace TrackMapper::Web

#endif // TESTWEBAPP_H
