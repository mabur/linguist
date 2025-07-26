#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define let __auto_type // Requires GNUC. C23 also has auto.

typedef double Vec3d __attribute__((vector_size(32))); // gcc/clang vector extensions

#define INIT_RANGE(range, count) do { \
    (range).first = malloc((count) * sizeof(*(range).first)); \
    (range).last = (range).first + (count); \
} while (0);

#define FREE_RANGE(range) do { \
    free((range).first); \
    (range).first = NULL; \
    (range).last = NULL; \
} while (0);

#define FOR_RANGE(it, range) \
    for (__auto_type it = (range).first; it != (range).last; ++it)


double dot(Vec3d a, Vec3d b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double squaredNorm(Vec3d v) {
    return dot(v, v);
}

double norm(Vec3d v) {
    return sqrt(squaredNorm(v));
}

Vec3d normalize(Vec3d v) {
    return v / norm(v);
}

typedef struct {
    Vec3d position;
    double squaredRadius;
    Vec3d color;
} Sphere;

typedef struct {
    Vec3d direction;
    Vec3d color;
} Light;

typedef struct {
    Sphere* first;
    Sphere* last;    
} Spheres;

typedef struct {
    Light* first;
    Light* last;
} Lights;

typedef struct {
    Spheres spheres;
    Lights lights;
    Vec3d atmosphere_color;
} World;

typedef struct {
    Vec3d position;
    Vec3d normal;
    double distance;
    Vec3d color;
} Intersection;

Intersection makeIntersection() {
    return (Intersection){.distance = INFINITY};
}

World makeWorld() {
    let R = 100000.0;
    let MAX_C = 1.0;
    let MIN_C = 0.1;
    let world = (World){};
    
    INIT_RANGE(world.spheres, 5);
    world.spheres.first[0] = (Sphere){(Vec3d){-2, 0, 6}, 1, (Vec3d){MAX_C, MAX_C, MIN_C}};
    world.spheres.first[1] = (Sphere){(Vec3d){0, 0, 5}, 1, (Vec3d){MAX_C, MIN_C, MIN_C}};
    world.spheres.first[2] = (Sphere){(Vec3d){2, 0, 4}, 1, (Vec3d){2 * MIN_C, 4 * MIN_C, MAX_C}};
    world.spheres.first[3] = (Sphere){(Vec3d){0, 1 + R, 0}, R * R, (Vec3d){MIN_C, MAX_C, MIN_C}};
    world.spheres.first[4] = (Sphere){(Vec3d){0, -1 - R, 0}, R * R, (Vec3d){MAX_C, MAX_C, MAX_C}};
    
    INIT_RANGE(world.lights, 2);
    world.lights.first[0] = (Light){(Vec3d){+1, +1, +2}, 0.4 * (Vec3d){1, 0.8, 0.5}};
    world.lights.first[1] = (Light){(Vec3d){-1, -1, -2}, 0.4 * (Vec3d){0.5, 0.5, 1}};
    
    world.atmosphere_color = 0.3 * (Vec3d){0.5, 0.5, 1};
    return world;
}

void freeWorld(World world) {
    FREE_RANGE(world.spheres);
    FREE_RANGE(world.lights);
}

Intersection findSingleIntersection(
    Vec3d start, Vec3d direction, Sphere sphere
) {
    let intersection = makeIntersection();
    let offset = sphere.position - start;
    let c = dot(direction, offset);
    if (c < 0.0) {
        return intersection;
    }
    let discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius;
    if (discriminant < 0.0) {
        return intersection;
    }
    intersection.distance = c - sqrt(discriminant);
    intersection.position = start + intersection.distance * direction;
    intersection.normal = normalize(intersection.position - sphere.position);
    intersection.color = sphere.color;
    return intersection;
}

Intersection findIntersection(Vec3d start, Vec3d direction, Spheres spheres) {
    let i1 = makeIntersection();
    FOR_RANGE(sphere, spheres) {
        let i2 = findSingleIntersection(start, direction, *sphere);
        if (i2.distance < i1.distance) {
            i1 = i2;
        }
    }
    return i1;
}

Vec3d shadeSingleLight(Intersection intersection, Light light) {
    let geometry = fmax(-dot(light.direction, intersection.normal), 0.0);
    return geometry * intersection.color * light.color;
}

Vec3d shadeAtmosphere(Intersection intersection, Vec3d atmosphere_color) {
    return sqrt(intersection.position[2]) * atmosphere_color;
}

Vec3d shade(Intersection intersection, World world) {
    if (isinf(intersection.distance)) {
        return (Vec3d){1, 1, 1};
    }
    let color = shadeAtmosphere(intersection, world.atmosphere_color);
    FOR_RANGE(light, world.lights) {
        color = color + shadeSingleLight(intersection, *light);
    }
    return color;
}

int colorU8fromF64(double c) {
    return (int)(fmin(255.0 * c, 255.0));
}

void writePixel(
    FILE* file,
    int x,
    int y,
    int width,
    int height,
    World world
) {
    let start = (Vec3d){0, 0, 0};
    let xd = (double)(x - width / 2);
    let yd = (double)(y - height / 2);
    let zd = (double)(height / 2);
    let direction = normalize((Vec3d){xd, yd, zd});
    let intersection = findIntersection(start, direction, world.spheres);
    let color = shade(intersection, world);
    let r = colorU8fromF64(color[0]);
    let g = colorU8fromF64(color[1]);
    let b = colorU8fromF64(color[2]);
    fprintf(file, "%d %d %d ", r, g, b);
}

void writeImage(const char* file_path, World world) {
    let file = fopen(file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "error opening file\n");
        exit(EXIT_FAILURE);
    }
    let WIDTH = 800;
    let HEIGHT = 600;
    fprintf(file, "%s\n%d\n%d\n%d\n", "P3", WIDTH, HEIGHT, 255);
    for (let y = 0; y < HEIGHT; ++y) {
        for (let x = 0; x < WIDTH; ++x) {
            writePixel(file, x, y, WIDTH, HEIGHT, world);
        }
    }
    fclose(file);
}

int main() {
    printf("Saving image\n");
    let world = makeWorld();
    writeImage("image.ppm", world);
    freeWorld(world);
    return 0;
}
