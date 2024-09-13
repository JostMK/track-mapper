//
// Created by Jost on 13/09/2024.
//

#include "interpolation.h"

namespace TrackMapper::Mesh {

    using Point = Raster::Point;

    double lerp(const double a, const double b, const double t) { return a + (b - a) * t; }

    double calculateCatmullRomT(const Point &p0, const Point &p1, const double t, const float alpha) {
        const double catmullRomfactor = pow((p1 - p0).SqLength(), alpha * .5f);
        return catmullRomfactor + t;
    }

    Point samplePathCatmullRomCurve(const Point &p0, const Point &p1, const Point &p2, const Point &p3, double t,
                                    const float alpha = .5f) {
        // modified based on https://en.wikipedia.org/wiki/Centripetal_Catmull–Rom_spline#Code_example_in_Unreal_C++
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

    std::vector<Point> interpolateCatmullRom(const std::vector<Point> &points, const int sampleCount,
                                             const float alpha) {
        double approxPathLength = 0;
        // first and last point are not interpolated with catmull-rom alg., so don't include in length calculation
        for (int i = 1; i < points.size() - 2; ++i) {
            const auto dist = points[i + 1] - points[i];
            approxPathLength += dist.Length();
        }
        const double distPerStep = approxPathLength / (sampleCount - 1);

        std::vector<Point> sampledPoints;

        int curSegmentIndex = 0;
        double curSegmentLength = (points[curSegmentIndex + 1] - points[curSegmentIndex + 2]).Length();
        double accumulatedLength = 0;
        for (int i = 0; i < sampleCount; ++i) {
            const auto t = static_cast<float>(accumulatedLength / curSegmentLength);
            const auto p0 = points[curSegmentIndex];
            const auto p1 = points[curSegmentIndex + 1];
            const auto p2 = points[curSegmentIndex + 2];
            const auto p3 = points[curSegmentIndex + 3];
            sampledPoints.push_back(samplePathCatmullRomCurve(p0, p1, p2, p3, t, alpha));

            accumulatedLength += distPerStep;
            if (accumulatedLength > curSegmentLength) {
                accumulatedLength -= curSegmentLength;
                curSegmentIndex++;
                curSegmentLength = (points[curSegmentIndex + 1] - points[curSegmentIndex + 2]).Length();
            }
        }

        return sampledPoints;
    }
} // namespace TrackMapper::Mesh
