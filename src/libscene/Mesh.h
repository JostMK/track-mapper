//
// Created by Jost on 19/09/2025.
//

#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

namespace LibScene {

    union Float3 {
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };

        Float3() : x(0), y(0), z(0) {}
        Float3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    };

    struct Face {
        int a, b, c;
    };

    enum Error {
        NONE,
        FAILED_TO_OPEN_OUTPUT_FILE,
    };

    struct Mesh {
        Float3 origin;

        std::vector<Float3> vertices;
        std::vector<Face> faces;

        [[nodiscard]] Error export_to_obj(const std::string &filepath) const;
    };

} // namespace LibScene

#endif // MESH_H
