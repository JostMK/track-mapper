
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

double lerp(const double a, const double b, const double t) { return a + (b - a) * t; }

double calculateCatmullRomT(const Point &p0, const Point &p1, const double t, const float alpha) {
    const auto [x, y, z] = p1 - p0;
    const double sqLength = x * x + y * y + z * z;
    const double catmullRomfactor = pow(sqLength, alpha * .5f);
    return catmullRomfactor + t;
}

Point samplePathCatmullRomCurve(const Point &p0, const Point &p1, const Point &p2, const Point &p3, double t,
                                const float alpha = .5f) {
    constexpr double t0 = .0;
    const double t1 = calculateCatmullRomT(p0, p1, t0, alpha);
    const double t2 = calculateCatmullRomT(p1, p2, t1, alpha);
    const double t3 = calculateCatmullRomT(p2, p3, t2, alpha);
    t = lerp(t1, t2, t);

    const Point A1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1;
    const Point A2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2;
    const Point A3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3;
    const Point B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2;
    const Point B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3;
    return (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2;
}

Point lerp(const Point &a, const Point &b, const double t) { return a + (b - a) * t; }

/**
 * @note For segment i (counting from 1) pass in point p[i-2], p[i-1], p[i], p[i+1] (access into p starting from 0)
 * @note For segment 1 (first segment) set isFirstSegment to true and pass in p[0], p[0], p[1], p[2]
 * @note For segment n (last segment) set isLastSegment to true and pass in p[n-2], p[n-1], p[n], p[n]
 */
Point samplePathBSplineCurve(const Point &p0, const Point &p1, const Point &p2, const Point &p3, const double t,
                             const bool isFirstSegment = false, const bool isLastSegment = false) {
    // following https://www.math.ucla.edu/~baker/149.1.02w/handouts/dd_splines.pdf chapter 4:
    const Point prevS = isFirstSegment ? p0 : 1. / 6 * p0 + 2. / 3 * p1 + 1. / 6 * p2;
    const Point curS = isLastSegment ? p3 : 1. / 6 * p1 + 2. / 3 * p2 + 1. / 6 * p3;

    const Point P1 = prevS;
    const Point P2 = 2. / 3 * p1 + 1. / 3 * p2;
    const Point P3 = 1. / 3 * p1 + 2. / 3 * p2;
    const Point P4 = curS;

    const Point A1 = lerp(P1, P2, t);
    const Point A2 = lerp(P2, P3, t);
    const Point A3 = lerp(P3, P4, t);
    const Point B1 = lerp(A1, A2, t);
    const Point B2 = lerp(A2, A3, t);

    return lerp(B1, B2, t);
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

void sampleCatmullRomWithConstPointsPerSegment(const std::vector<Point> &points, const int pointsPerSegment = 10) {
    std::vector<Point> sampledPoints;

    for (int i = 0; i < points.size() - 3; ++i) {
        for (int j = 1; j <= pointsPerSegment; ++j) {
            const float t = static_cast<float>(j) / static_cast<float>(pointsPerSegment);
            sampledPoints.push_back(
                    samplePathCatmullRomCurve(points[i], points[i + 1], points[i + 2], points[i + 3], t));
        }
    }

    std::cout << "Specify obj file for path with " << pointsPerSegment
              << " samples per segment: (enter none to skip step)" << std::endl;
    std::string filePath;
    std::getline(std::cin, filePath);

    if (!filePath.empty())
        writeToObj(sampledPoints, filePath);
}

void sampleCatmullRomWithEquallySpacedPoints(const std::vector<Point> &points, const int samplingCount = 100) {
    double approxPathLength = 0;
    // first and last point are not interpolated wit catmull-rom alg. -> seams unpraktical for our use case
    for (int i = 1; i < points.size() - 2; ++i) {
        const auto dist = points[i + 1] - points[i];
        approxPathLength += dist.Length();
    }
    const double distPerStep = approxPathLength / (samplingCount - 1);

    std::vector<Point> sampledPoints;

    int curSegmentIndex = 0;
    double curSegmentLength = (points[curSegmentIndex + 1] - points[curSegmentIndex + 2]).Length();
    double accumulatedLength = 0;
    for (int i = 0; i < samplingCount; ++i) {
        const auto t = static_cast<float>(accumulatedLength / curSegmentLength);
        sampledPoints.push_back(samplePathCatmullRomCurve(points[curSegmentIndex], points[curSegmentIndex + 1],
                                                          points[curSegmentIndex + 2], points[curSegmentIndex + 3], t));

        accumulatedLength += distPerStep;
        if (accumulatedLength > curSegmentLength) {
            accumulatedLength -= curSegmentLength;
            curSegmentIndex++;
            curSegmentLength = (points[curSegmentIndex + 1] - points[curSegmentIndex + 2]).Length();
        }
    }

    std::cout << "Specify obj file for path with " << samplingCount << " samples overall: (enter none to skip step)"
              << std::endl;
    std::string filePath;
    std::getline(std::cin, filePath);

    if (!filePath.empty())
        writeToObj(sampledPoints, filePath);
}

void sampleBSplineWithConstPointsPerSegment(const std::vector<Point> &points, const int pointsPerSegment = 10) {
    std::vector<Point> sampledPoints;

    for (int i = 1; i < points.size(); ++i) {
        // iterates over segments
        const bool isFirstSegment = i == 1;
        const bool isLastSegment = i == points.size()-1;
        Point p0 = isFirstSegment ? points[0] : points[i - 2];
        Point p1 = points[i - 1];
        Point p2 = points[i];
        Point p3 = isLastSegment ? points[points.size()-1] : points[i + 1];

        for (int j = 1; j <= pointsPerSegment; ++j) { // iterates over points in segment
            const float t = static_cast<float>(j) / static_cast<float>(pointsPerSegment);
            sampledPoints.push_back(samplePathBSplineCurve(p0, p1, p2, p3, t, isFirstSegment, isLastSegment));
        }
    }

    std::cout << "Specify obj file for path with " << pointsPerSegment
              << " samples per segment: (enter none to skip step)" << std::endl;
    std::string filePath;
    std::getline(std::cin, filePath);

    if (!filePath.empty())
        writeToObj(sampledPoints, filePath);
}

void sampleBSplineWithEquallySpacedPoints(const std::vector<Point> &points, const int samplingCount = 100) {
    double approxPathLength = 0;
    for (int i = 0; i < points.size() - 1; ++i) {
        const auto dist = points[i + 1] - points[i];
        approxPathLength += dist.Length();
    }
    const double distPerStep = approxPathLength / (samplingCount - 1);

    std::vector<Point> sampledPoints;

    int curSegmentIndex = 1;
    double curSegmentLength = (points[curSegmentIndex - 1] - points[curSegmentIndex]).Length();
    double accumulatedLength = 0;
    for (int i = 0; i < samplingCount; ++i) {
        const auto t = static_cast<float>(accumulatedLength / curSegmentLength);

        const bool isFirstSegment = curSegmentIndex == 1;
        const bool isLastSegment = curSegmentIndex == points.size()-1;
        Point p0 = isFirstSegment ? points[0] : points[curSegmentIndex - 2];
        Point p1 = points[curSegmentIndex - 1];
        Point p2 = points[curSegmentIndex];
        Point p3 = isLastSegment ? points[points.size()-1] : points[curSegmentIndex + 1];
        sampledPoints.push_back(samplePathBSplineCurve(p0, p1, p2, p3, t, isFirstSegment, isLastSegment));

        accumulatedLength += distPerStep;
        if (accumulatedLength > curSegmentLength) {
            accumulatedLength -= curSegmentLength;
            curSegmentIndex++;
            curSegmentLength = (points[curSegmentIndex - 1] - points[curSegmentIndex]).Length();
        }
    }

    std::cout << "Specify obj file for path with " << samplingCount << " samples overall: (enter none to skip step)"
              << std::endl;
    std::string filePath;
    std::getline(std::cin, filePath);

    if (!filePath.empty())
        writeToObj(sampledPoints, filePath);
}

int main() {
    std::cout << "Enter Path to path csv file:" << std::endl;
    std::string inFilePath;
    std::cin >> inFilePath;

    std::cout << "Add random hight data to path: [y]es, [n]o" << std::endl;
    std::string randomHightSelection;
    std::cin >> randomHightSelection;
    const bool useRandomHight = randomHightSelection[0] == 'y';

    std::vector<Point> points;
    if (std::ifstream pathFile(inFilePath); pathFile.is_open()) {
        std::string line;
        while (pathFile.good()) {
            std::getline(pathFile, line);
            const size_t seperator = line.find_first_of(';');
            const double lat = std::stod(line.substr(0, seperator));
            const double lng = std::stod(line.substr(seperator + 1));
            const double hight = useRandomHight ? rand() / (RAND_MAX / 0.0001) : 0; // NOLINT(*-msc50-cpp)
            points.emplace_back(lat, hight, lng);
        }
        pathFile.close();
    }

    {
        std::cin.ignore();
        std::cout << "Specify obj file for original path: (enter none to skip step)" << std::endl;
        std::string filePath;
        std::getline(std::cin, filePath);

        if (!filePath.empty())
            writeToObj(points, filePath);
    }

    std::cout << "\nInterpolating path using Catmull-Rom.." << std::endl;

    sampleCatmullRomWithConstPointsPerSegment(points, 10);

    sampleCatmullRomWithEquallySpacedPoints(points, 1000);

    std::cout << "\nInterpolating path using B-spline.." << std::endl;

    sampleBSplineWithConstPointsPerSegment(points, 10);

    sampleBSplineWithEquallySpacedPoints(points, 1000);
}
