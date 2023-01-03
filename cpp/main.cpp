#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct Vec3d {
    double x;
    double y;
    double z;
};

Vec3d operator-(const Vec3d& a, const Vec3d& b) {
    return Vec3d{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3d operator*(double a, const Vec3d& b) {
    return Vec3d{a * b.x, a * b.y, a * b.z};
}

double dot(const Vec3d& a, const Vec3d& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double squaredNorm(const Vec3d& v) {
    return dot(v, v);
}

double norm(const Vec3d& v) {
    return sqrt(squaredNorm(v));
}

Vec3d normalize(const Vec3d& v) {
    return 1.0 / norm(v) * v;
}

struct Sphere {
    Vec3d position;
    double squaredRadius;
};

Sphere makeSphere() {
    return Sphere{Vec3d{0, 0, 5}, 1};
}

double findIntersection(
    const Vec3d& start, const Vec3d& direction, const Sphere& sphere
) {
    const auto offset = sphere.position - start;
    const auto c = dot(direction, offset);
    if (c < 0.0) return INFINITY;
    const auto discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius;
    if (discriminant < 0.0) return INFINITY;
    return c - std::sqrt(discriminant);
}

void writeImage(const std::string& file_path, const Sphere& sphere) {
    using namespace std;
    ofstream file(file_path);
    const auto width = 320;
    const auto height = 240;
    const auto focal_length = height / 2;
    file << "P3" << endl << width << " " << height << endl << 255 << endl;
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            const auto start = Vec3d{0, 0, 0};
            const auto xd = double(x - width / 2);
            const auto yd = double(y - height / 2);
            const auto zd = double(focal_length);
            const auto direction = normalize(Vec3d{xd, yd, zd});
            const auto intersection = findIntersection(start, direction, sphere);
            if (std::isfinite(intersection)) {
                const auto color = std::max(int(intersection), 255);
                file
                    << color << " "
                    << color << " "
                    << color << " ";
            }
            else {
                file
                    << 0 << " "
                    << 0 << " "
                    << 0 << " ";
            }
        }
    }
    file.close();
}

int main() {
    using namespace std;
    cout << "Saving image" << endl;
    const auto spheres = makeSphere();
    writeImage("image.ppm", spheres);
}
