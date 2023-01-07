#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Vec3d {
    double x = 0;
    double y = 0;
    double z = 0;
};

Vec3d operator+(Vec3d a, Vec3d b) {
    return Vec3d{a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3d operator-(Vec3d a, Vec3d b) {
    return Vec3d{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3d operator*(Vec3d a, Vec3d b) {
    return Vec3d{ a.x * b.x, a.y * b.y, a.z * b.z };
}

Vec3d operator*(double a, Vec3d b) {
    return Vec3d{a * b.x, a * b.y, a * b.z};
}

double dot(Vec3d a, Vec3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double squaredNorm(Vec3d v) {
    return dot(v, v);
}

double norm(Vec3d v) {
    return sqrt(squaredNorm(v));
}

Vec3d normalize(Vec3d v) {
    return 1.0 / norm(v) * v;
}

struct Sphere {
    Vec3d position;
    double squaredRadius;
    Vec3d color;
};

struct Light {
    Vec3d direction;
    Vec3d color;
};

struct World {
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    Vec3d atmosphere_color;
};

struct Intersection {
    Vec3d position = {};
    Vec3d normal = {};
    double distance = INFINITY;
    Vec3d color = {};
};

bool operator<(Intersection a, Intersection b) {
    return a.distance < b.distance;
}

World makeWorld() {
    const auto R = 100000.0;
    const auto MAX_C = 1.0;
    const auto MIN_C = 0.1;
    auto world = World{};
    world.spheres = {
        Sphere{Vec3d{-2, 0, 6}, 1, Vec3d{MAX_C, MAX_C, MIN_C}},
        Sphere{Vec3d{0, 0, 5}, 1, Vec3d{MAX_C, MIN_C, MIN_C}},
        Sphere{Vec3d{2, 0, 4}, 1, Vec3d{2 * MIN_C, 4 * MIN_C, MAX_C}},
        Sphere{Vec3d{0, 1 + R, 0}, R * R, Vec3d{MIN_C, MAX_C, MIN_C}},
        Sphere{Vec3d{0, -1 - R, 0}, R * R, Vec3d{MAX_C, MAX_C, MAX_C}},
    };
    world.lights = {
        Light{Vec3d{+1, +1, +2}, 0.4 * Vec3d{1, 0.8, 0.5}},
        Light{Vec3d{-1, -1, -2}, 0.4 * Vec3d{0.5, 0.5, 1}},
    };
    world.atmosphere_color = 0.3 * Vec3d{0.5, 0.5, 1};
    return world;
}

Intersection findSingleIntersection(
    Vec3d start, Vec3d direction, Sphere sphere
) {
    const auto offset = sphere.position - start;
    const auto c = dot(direction, offset);
    if (c < 0.0) return Intersection{};
    const auto discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius;
    if (discriminant < 0.0) return Intersection{};
    auto intersection = Intersection{};
    intersection.distance = c - sqrt(discriminant);
    intersection.position = start + intersection.distance * direction;
    intersection.normal = normalize(intersection.position - sphere.position);
    intersection.color = sphere.color;
    return intersection;
}

Intersection findIntersection(
    Vec3d start, Vec3d direction, const std::vector<Sphere>& spheres
) {
    auto intersection = Intersection{};
    for (const auto& sphere : spheres) {
        intersection = std::min(intersection, findSingleIntersection(start, direction, sphere));
    }
    return intersection;
}

Vec3d shadeSingleLight(Intersection intersection, Light light) {
    const auto geometry = std::max(-dot(light.direction, intersection.normal), 0.0);
    return geometry * intersection.color * light.color;
}

Vec3d shadeAtmosphere(Intersection intersection, Vec3d atmosphere_color) {
    return sqrt(intersection.position.z) * atmosphere_color;
}

Vec3d shade(Intersection intersection, World world) {
    if (isinf(intersection.distance)) {
        return Vec3d{ 1, 1, 1 };
    }
    auto color = shadeAtmosphere(intersection, world.atmosphere_color);
    for (const auto& light : world.lights) {
        color = color + shadeSingleLight(intersection, light);
    }
    return color;
}

int colorU8fromF64(double c) {
    return int(std::min(255.0 * c, 255.0));
}

void writePixel(
    std::ofstream& file,
    int x,
    int y,
    int width,
    int height,
    const World& world
) {
    const auto start = Vec3d{0, 0, 0};
    const auto xd = double(x - width / 2);
    const auto yd = double(y - height / 2);
    const auto zd = double(height / 2);
    const auto direction = normalize(Vec3d{xd, yd, zd});
    const auto intersection = findIntersection(start, direction, world.spheres);
    const auto color = shade(intersection, world);
    const auto r = colorU8fromF64(color.x);
    const auto g = colorU8fromF64(color.y);
    const auto b = colorU8fromF64(color.z);
    file << r << " " << g << " " << b << " ";
}

void writeImage(const std::string& file_path, const World& world) {
    using namespace std;
    ofstream file(file_path);
    const auto WIDTH = 800;
    const auto HEIGHT = 600;
    file << "P3" << endl << WIDTH << " " << HEIGHT << endl << 255 << endl;
    for (auto y = 0; y < HEIGHT; ++y) {
        for (auto x = 0; x < WIDTH; ++x) {
            writePixel(file, x, y, WIDTH, HEIGHT, world);
        }
    }
    file.close();
}

int main() {
    using namespace std;
    cout << "Saving image" << endl;
    const auto world = makeWorld();
    writeImage("image.ppm", world);
}
