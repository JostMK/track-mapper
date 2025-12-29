//
// Created by Jost on 19/09/2025.
//

#include "Mesh.h"

#include <fstream>

namespace LibScene {
    Error Mesh::export_to_obj(const std::string &filepath) const {

        std::ofstream outputFile;

        outputFile.open(filepath);

        if (!outputFile.is_open()) {
            return Error::FAILED_TO_OPEN_OUTPUT_FILE;
        }

        for (const auto &vertex: vertices) {
            outputFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
        }

        for (const auto &[a, b, c]: faces) {
            // vertex indices in obj start with 1
            outputFile << "f " << a + 1 << " " << b + 1 << " " << c + 1 << "\n";
        }

        outputFile.flush();
        outputFile.close();
        return Error::NONE;
    }
} // namespace LibScene
