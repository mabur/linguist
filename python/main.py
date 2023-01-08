import math
from dataclasses import dataclass
from typing import Iterable

@dataclass
class Vec3d:
    x: float
    y: float
    z: float

    def __add__(self, right):
        return Vec3d(self.x + right.x, self.y + right.y, self.z + right.z)

    def __sub__(self, right):
        return Vec3d(self.x - right.x, self.y - right.y, self.z - right.z)

    def __mul__(self, right):
        if isinstance(right, Vec3d):
            return Vec3d(self.x * right.x, self.y * right.y, self.z * right.z)
        else:
            return Vec3d(self.x * right, self.y * right, self.z * right)

    def __rmul__(self, left):
        if isinstance(left, Vec3d):
            return Vec3d(self.x * left.x, self.y * left.y, self.z * left.z)
        else:
            return Vec3d(self.x * left, self.y * left, self.z * left)


def dot(a: Vec3d, b: Vec3d) -> float:
    return a.x * b.x + a.y * b.y + a.z * b.z


def squaredNorm(v: Vec3d) -> float:
    return dot(v, v)


def norm(v: Vec3d) -> float:
    return math.sqrt(squaredNorm(v))


def normalize(v: Vec3d) -> Vec3d:
    return (1.0 / norm(v)) * v


@dataclass
class Sphere:
    position: Vec3d
    squaredRadius: float
    color: Vec3d


@dataclass
class Light:
    direction: Vec3d
    color: Vec3d


@dataclass
class World:
    spheres: Iterable[Sphere]
    lights: Iterable[Light]
    atmosphere_color: Vec3d


@dataclass
class Intersection:
    position: Vec3d = Vec3d(0, 0, 0)
    normal: Vec3d = Vec3d(0, 0, 0)
    color: Vec3d = Vec3d(1, 1, 1)
    distance: float = math.inf


def makeWorld() -> World:
    R = 100000.0
    MAX_C = 1.0
    MIN_C = 0.1
    spheres = [
        Sphere(Vec3d(-2, 0, 6), 1, Vec3d(MAX_C, MAX_C, MIN_C)),
        Sphere(Vec3d(0, 0, 5), 1, Vec3d(MAX_C, MIN_C, MIN_C)),
        Sphere(Vec3d(2, 0, 4), 1, Vec3d(2 * MIN_C, 4 * MIN_C, MAX_C)),
        Sphere(Vec3d(0, 1 + R, 0), R * R, Vec3d(MIN_C, MAX_C, MIN_C)),
        Sphere(Vec3d(0, -1 - R, 0), R * R, Vec3d(MAX_C, MAX_C, MAX_C)),
    ]
    lights = [
        Light(Vec3d(+1, +1, +2), 0.4 * Vec3d(1, 0.8, 0.5)),
        Light(Vec3d(-1, -1, -2), 0.4 * Vec3d(0.5, 0.5, 1)),
    ]
    atmosphere_color = 0.3 * Vec3d(0.5, 0.5, 1)
    world = World(spheres=spheres, lights=lights, atmosphere_color=atmosphere_color)
    return world


def findSingleIntersection(start: Vec3d, direction: Vec3d, sphere: Sphere) -> Intersection:
    intersection = Intersection()
    offset = sphere.position - start
    c = dot(direction, offset)
    if c < 0.0:
        return intersection
    discriminant = c * c - squaredNorm(offset) + sphere.squaredRadius
    if discriminant < 0.0:
        return intersection
    intersection.distance = c - math.sqrt(discriminant)
    intersection.position = start + intersection.distance * direction
    intersection.normal = normalize(intersection.position - sphere.position)
    intersection.color = sphere.color
    return intersection


def findIntersection(start: Vec3d, direction: Vec3d, spheres: Iterable[Sphere]) -> Intersection:
    return min(
        (findSingleIntersection(start, direction, sphere) for sphere in spheres),
        key=lambda i: i.distance
    )


def shadeSingleLight(intersection: Intersection, light: Light) -> Vec3d:
    geometry = max(-dot(light.direction, intersection.normal), 0.0)
    return geometry * (intersection.color * light.color)


def shadeAtmosphere(intersection: Intersection, atmosphere_color: Vec3d) -> Vec3d:
    return math.sqrt(intersection.position.z) * atmosphere_color


def shade(intersection: Intersection, world: World) -> Vec3d:
    if math.isinf(intersection.distance):
        return Vec3d(1, 1, 1)
    color = shadeAtmosphere(intersection, world.atmosphere_color)
    return sum((shadeSingleLight(intersection, light) for light in world.lights), color)


def colorU8fromF64(c: float) -> int:
    return int(min(255.0 * c, 255.0))


def serializePixel(x: int, y: int, width: int, height: int, world: World) -> str:
    start = Vec3d(0, 0, 0)
    xd = x - width / 2.0
    yd = y - height / 2.0
    zd = height / 2.0
    direction = normalize(Vec3d(xd, yd, zd))
    intersection = findIntersection(start, direction, world.spheres)
    color = shade(intersection, world)
    r = colorU8fromF64(color.x)
    g = colorU8fromF64(color.y)
    b = colorU8fromF64(color.z)
    return "{} {} {} ".format(r, g, b)


def writeImage(file_path: str, world: World) -> None:
    WIDTH = 800
    HEIGHT = 600
    with open(file_path, 'w') as file:
        file.write("{}\n{}\n{}\n{}\n".format("P3", WIDTH, HEIGHT, 255))
        file.write(
            "".join(
                serializePixel(x, y, WIDTH, HEIGHT, world)
                for y in range(HEIGHT) for x in range(WIDTH)
            )
        )


def main():
    print("Saving image")
    world = makeWorld()
    writeImage("image.ppm", world)


if __name__ == "__main__":
    main()
