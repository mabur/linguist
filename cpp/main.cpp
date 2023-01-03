#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct Vec3d {
    double x = 0;
    double y = 0;
    double z = 0;
};

Vec3d operator+(const Vec3d& a, const Vec3d& b) {
    return Vec3d{a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3d operator-(const Vec3d& a, const Vec3d& b) {
    return Vec3d{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3d operator*(const Vec3d& a, const Vec3d& b) {
    return Vec3d{ a.x * b.x, a.y * b.y, a.z * b.z };
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
    Vec3d color;
};

struct Light {
    Vec3d position;
    Vec3d color;
};

struct Intersection {
    Vec3d position = {};
    Vec3d normal = {};
    double distance = INFINITY;
    Vec3d color = {};
};

std::vector<Sphere> makeSpheres() {
    return {
        Sphere{Vec3d{0, 0, 5}, 1, Vec3d{1, 0, 0}},
        Sphere{Vec3d{0, 1+1000, 0}, 1000*1000, Vec3d{1, 1, 1}},
    };
}

Intersection findIntersection(
    const Vec3d& start, const Vec3d& direction, const Sphere& sphere
) {
    const auto offset = sphere.position - start;
    const auto c = dot(direction, offset);
    if (c < 0.0) return Intersection{};
    const auto discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius;
    if (discriminant < 0.0) return Intersection{};
    auto intersection = Intersection{};
    intersection.distance = c - std::sqrt(discriminant);
    intersection.position = start + intersection.distance * direction;
    intersection.normal = normalize(intersection.position - sphere.position);
    intersection.color = sphere.color;
    return intersection;
}

int colorU8fromF64(double c) {
    return int(std::min(255.0 * c, 255.0));
}

Vec3d shade(const Intersection& intersection, const Light& light) {
    const auto offset = light.position - intersection.position;
    const auto c = dot(offset, intersection.normal);
    if (c < 0) return Vec3d{};
    const auto geometry = c / squaredNorm(offset);
    return geometry * intersection.color * light.color;
}

void writeImage(const std::string& file_path, const std::vector<Sphere>& spheres, const Light& light) {
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
            auto closest_intersection = Intersection{};
            for (const auto& sphere : spheres) {
                const auto intersection = findIntersection(start, direction, sphere);
                if (intersection.distance < closest_intersection.distance) {
                    closest_intersection = intersection;
                }
            }

            if (std::isfinite(closest_intersection.distance)) {
                const auto color = shade(closest_intersection, light);
                const auto r = colorU8fromF64(color.x);
                const auto g = colorU8fromF64(color.y);
                const auto b = colorU8fromF64(color.z);
                file << r << " " << g << " " << b << " ";
            }
            else {
                file << 0 << " " << 0 << " " << 0 << " ";
            }
        }
    }
    file.close();
}

int main() {
    using namespace std;
    cout << "Saving image" << endl;
    const auto spheres = makeSpheres();
    const auto light = Light{Vec3d{0, -10, 0}, 10 * Vec3d{1,1,1}};
    writeImage("image.ppm", spheres, light);
}
