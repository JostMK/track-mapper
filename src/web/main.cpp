//
// Created by Jost on 24/05/2024.
//

#include <iostream>
#include "BasicWebApp.h"

void TestWebApp();

int main() {
    TestWebApp();
    return 0;
}

void TestWebApp() {
    std::cout << "Enter Path to fmi file:" << std::endl;

    std::string filePath;
    std::cin >> filePath;

    try {
        TrackMapper::Web::BasicWebApp app(filePath);
        app.Start();
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "An unknown error happend :(" << std::endl;
    }
}
