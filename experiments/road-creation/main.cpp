
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Point {
    double x, y, z;

    Point operator-() const { return Point{-x, -y, -z}; }
    Point operator+(const Point &other) const { return Point{x + other.x, y + other.y, z + other.z}; }
    Point operator-(const Point &other) const { return Point{x - other.x, y - other.y, z - other.z}; }
    Point operator*(const double factor) const { return Point{x * factor, y * factor, z * factor}; }

    [[nodiscard]] double Length() const { return sqrt(x * x + y * y + z * z); }
};
Point operator*(const double factor, const Point &point) {
    return Point{point.x * factor, point.y * factor, point.z * factor};
}

void writeToObj(const std::vector<Point> &points, const std::string &filePath) {
    std::ofstream objFile;
    objFile.open(filePath);

    if (!objFile.is_open()) {
        std::cout << "Failed to open file: '" << filePath << "'" << std::endl;
        return;
    }

    for (auto [x, y, z]: points) {
        objFile << std::format("v {} {} {}\n", x, y, z);
    }

    objFile << std::endl;
    objFile.flush();
    objFile.close();
}

void createRoad(const std::vector<Point> &points, const double width) {
    std::vector<Point> roadPoints;
    for (int i = 1; i < points.size() - 1; ++i) {
        Point v1 = points[i - 1] - points[i];
        Point v2 = points[i + 1] - points[i];
        Point n1 = v1 * (1. / v1.Length());
        Point n2 = v2 * (1. / v2.Length());
        Point h = n1+n2;
        Point nh = h * (1. / h.Length());

        roadPoints.push_back(points[i] + nh * width);
        roadPoints.push_back(points[i] - nh * width);
    }

    std::cin.ignore();
    std::cout << "Specify obj file for road points: " << std::endl;
    std::string filePath;
    std::getline(std::cin, filePath);

    if (!filePath.empty())
        writeToObj(roadPoints, filePath);
}

int main() {
    std::cout << "Enter Path to path csv file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::vector<Point> points;
    if (std::ifstream pathFile(inFilePath); pathFile.is_open()) {
        std::string line;
        while (pathFile.good()) {
            std::getline(pathFile, line);
            const size_t seperator = line.find_first_of(';');
            const double lat = std::stod(line.substr(0, seperator));
            const double lng = std::stod(line.substr(seperator + 1));
            points.emplace_back(lat, 0, lng);
        }
        pathFile.close();
    }

    createRoad(points, 0.00007);
}
