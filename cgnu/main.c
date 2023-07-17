#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define let __auto_type

typedef struct {
    double x;
    double y;
    double z;
} Vec3d;

Vec3d add(Vec3d a, Vec3d b) {
    return (Vec3d){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3d sub(Vec3d a, Vec3d b) {
    return (Vec3d){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3d mul(Vec3d a, Vec3d b) {
    return (Vec3d){ a.x * b.x, a.y * b.y, a.z * b.z };
}

Vec3d muls(double a, Vec3d b) {
    return (Vec3d){a * b.x, a * b.y, a * b.z};
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
    return muls(1.0 / norm(v), v);
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

    let num_spheres = 5;
    world.spheres.first = malloc(num_spheres * sizeof(Sphere));
    world.spheres.last = world.spheres.first + num_spheres;
    world.spheres.first[0] = (Sphere){(Vec3d){-2, 0, 6}, 1, (Vec3d){MAX_C, MAX_C, MIN_C}};
    world.spheres.first[1] = (Sphere){(Vec3d){0, 0, 5}, 1, (Vec3d){MAX_C, MIN_C, MIN_C}};
    world.spheres.first[2] = (Sphere){(Vec3d){2, 0, 4}, 1, (Vec3d){2 * MIN_C, 4 * MIN_C, MAX_C}};
    world.spheres.first[3] = (Sphere){(Vec3d){0, 1 + R, 0}, R * R, (Vec3d){MIN_C, MAX_C, MIN_C}};
    world.spheres.first[4] = (Sphere){(Vec3d){0, -1 - R, 0}, R * R, (Vec3d){MAX_C, MAX_C, MAX_C}};

    let num_lights = 2;
    world.lights.first = malloc(num_lights * sizeof(Light));
    world.lights.last = world.lights.first + num_lights;
    world.lights.first[0] = (Light){(Vec3d){+1, +1, +2}, muls(0.4, (Vec3d){1, 0.8, 0.5})};
    world.lights.first[1] = (Light){(Vec3d){-1, -1, -2}, muls(0.4, (Vec3d){0.5, 0.5, 1})};
    
    world.atmosphere_color = muls(0.3, (Vec3d){0.5, 0.5, 1});
    return world;
}

void freeWorld(World world) {
    free(world.spheres.first);
    free(world.lights.first);
    world.spheres.first = NULL;
    world.spheres.last = NULL;
    world.lights.first = NULL;
    world.lights.last = NULL;
}

Intersection findSingleIntersection(
    Vec3d start, Vec3d direction, Sphere sphere
) {
    let intersection = makeIntersection();
    let offset = sub(sphere.position, start);
    let c = dot(direction, offset);
    if (c < 0.0) {
        return intersection;
    }
    let discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius;
    if (discriminant < 0.0) {
        return intersection;
    }
    intersection.distance = c - sqrt(discriminant);
    intersection.position = add(start, muls(intersection.distance, direction));
    intersection.normal = normalize(sub(intersection.position, sphere.position));
    intersection.color = sphere.color;
    return intersection;
}

Intersection findIntersection(
    Vec3d start, Vec3d direction, const Sphere* spheres_first, const Sphere* spheres_last
) {
    let i1 = makeIntersection();
    for (let s = spheres_first; s != spheres_last; ++s) {
        let i2 = findSingleIntersection(start, direction, *s);
        if (i2.distance < i1.distance) {
            i1 = i2;
        }
    }
    return i1;
}

Vec3d shadeSingleLight(Intersection intersection, Light light) {
    let geometry = MAX(-dot(light.direction, intersection.normal), 0.0);
    return muls(geometry, mul(intersection.color, light.color));
}

Vec3d shadeAtmosphere(Intersection intersection, Vec3d atmosphere_color) {
    return muls(sqrt(intersection.position.z), atmosphere_color);
}

Vec3d shade(Intersection intersection, World world) {
    if (isinf(intersection.distance)) {
        return (Vec3d){ 1, 1, 1 };
    }
    let color = shadeAtmosphere(intersection, world.atmosphere_color);
    for (let light = world.lights.first; light != world.lights.last; ++light) {
        color = add(color, shadeSingleLight(intersection, *light));
    }
    return color;
}

int colorU8fromF64(double c) {
    return (int)(MIN(255.0 * c, 255.0));
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
    let intersection = findIntersection(start, direction, world.spheres.first, world.spheres.last);
    let color = shade(intersection, world);
    let r = colorU8fromF64(color.x);
    let g = colorU8fromF64(color.y);
    let b = colorU8fromF64(color.z);
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
