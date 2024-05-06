//
// Created by Jost on 02/05/2024.
//

#include "TestWebApp.h"

#include "crow.h"

void TestWebApp::Start() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello world";
    });

    app.port(18080).multithreaded().run();
}
