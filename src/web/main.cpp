//
// Created by Jost on 24/05/2024.
//

#include <iostream>

#include "TestWebApp.h"
#include "../graph/FMIGraphreader.h"
#include "../graph/BasicGraph.h"

void TestWebApp();

int main()
{
    TestWebApp();
    return 0;
}

void TestWebApp() {
    std::cout << "Enter Path to fmi file:" << std::endl;

    std::string filePath;
    std::cin>>filePath;
    const BasicGraph graph = FMIGraphReader::read(filePath);

    TrackMapper::Web::TestWebApp::Start(graph);
}