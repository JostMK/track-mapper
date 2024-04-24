//
// Created by Jost on 22/04/2024.
//

#ifndef FMIGRAPHREADER_H
#define FMIGRAPHREADER_H
#include <string>

#include "BasicGraph.h"

class FMIGraphReader {
public:
    static BasicGraph read(std::string &filePath);
};


#endif //FMIGRAPHREADER_H
