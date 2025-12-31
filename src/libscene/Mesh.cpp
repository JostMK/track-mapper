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

        constexpr auto buffer_size = 67108864; // 64 MiB
        std::string buffer;
        buffer.reserve(buffer_size);

        for (const auto &vertex: vertices) {
            buffer += "v " + std::to_string(vertex.x) + " " + std::to_string(vertex.y) + " " +
                      std::to_string(vertex.z) + "\n";

            if (const int saturation = static_cast<int>(buffer.size()); saturation >= buffer_size * 0.9) {
                outputFile.write(buffer.data(), saturation);
                buffer = "";
            }
        }

        for (const auto &[a, b, c]: faces) {
            // vertex indices in obj start with 1
            buffer += "f " + std::to_string(a + 1) + " " + std::to_string(b + 1) + " " + std::to_string(c + 1) + "\n";

            if (const int saturation = static_cast<int>(buffer.size()); saturation >= buffer_size * 0.9) {
                outputFile.write(buffer.data(), saturation);
                buffer = "";
            }
        }

        outputFile.write(buffer.data(), static_cast<int>(buffer.size()));
        outputFile.flush();

        outputFile.close();
        return Error::NONE;
    }
} // namespace LibScene
