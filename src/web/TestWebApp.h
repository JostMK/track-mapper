//
// Created by Jost on 02/05/2024.
//

#ifndef TESTWEBAPP_H
#define TESTWEBAPP_H
#include "../graph/BasicGraph.h"

namespace TrackMapper::Web {
    class TestWebApp {
    public:
        static void Start(const BasicGraph &graph);
    };
} // namespace TrackMapper::Web

#endif // TESTWEBAPP_H
