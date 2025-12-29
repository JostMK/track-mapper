//
// Created by Jost on 19/09/2025.
//

#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

namespace LibScene {

    union Double3 {
        struct {
            double x, y, z;
        };
        struct {
            double r, g, b;
        };

        Double3() : x(0), y(0), z(0) {}
        Double3(const double x, const double y, const double z) : x(x), y(y), z(z) {}
    };

    struct Face {
        int a, b, c;
    };

    enum Error {
        NONE,
        FAILED_TO_OPEN_OUTPUT_FILE,
    };

    struct Mesh {
        Double3 origin;

        std::vector<Double3> vertices;
        std::vector<Face> faces;

        [[nodiscard]] Error export_to_obj(const std::string &filepath) const;
    };

} // namespace LibScene

#endif // MESH_H
