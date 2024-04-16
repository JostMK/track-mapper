#include <iostream>

#include "src/graphs/FMIGraph.h"

int main()
{
    std::cout << "Hello, World!" << std::endl;

    FMIGraph graph;
    graph.Test();
    std::cout << "Test2: " << graph.Test2() << std::endl;

    return 0;
}
