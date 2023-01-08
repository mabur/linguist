from dataclasses import dataclass
import math
from typing import List, Sequence

@dataclass
class Vec3d:
	x: float
	y: float
	z: float

def add(a: Vec3d, b: Vec3d) -> Vec3d:
	return Vec3d(a.x + b.x, a.y + b.y, a.z + b.z)

def sub(a: Vec3d, b: Vec3d) -> Vec3d:
	return Vec3d(a.x - b.x, a.y - b.y, a.z - b.z)

def mul(a: Vec3d, b: Vec3d) -> Vec3d:
	return Vec3d(a.x * b.x, a.y * b.y, a.z * b.z)

def muls(a: float, b: Vec3d) -> Vec3d:
	return Vec3d(a * b.x, a * b.y, a * b.z)

def dot(a: Vec3d, b: Vec3d) -> float:
	return a.x*b.x + a.y*b.y + a.z*b.z

def squaredNorm(v: Vec3d) -> float:
	return dot(v, v)

def norm(v: Vec3d) -> float:
	return math.sqrt(squaredNorm(v))

def normalize(v: Vec3d) -> Vec3d:
	return muls(1.0/norm(v), v)

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
	spheres: Sequence[Sphere]
	lights: Sequence[Light]
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
        Light(Vec3d(+1, +1, +2), muls(0.4, Vec3d(1, 0.8, 0.5))),
        Light(Vec3d(-1, -1, -2), muls(0.4, Vec3d(0.5, 0.5, 1))),
    ]
    atmosphere_color = muls(0.3, Vec3d(0.5, 0.5, 1))
    world = World(spheres=spheres, lights=lights, atmosphere_color=atmosphere_color)
    return world

def findSingleIntersection(start: Vec3d, direction: Vec3d, sphere: Sphere) -> Intersection:
	intersection = Intersection()
	offset = sub(sphere.position, start)
	c = dot(direction, offset)
	if c < 0.0:
		return intersection
	discriminant = c*c - squaredNorm(offset) + sphere.squaredRadius
	if discriminant < 0.0:
		return intersection
	intersection.distance = c - math.sqrt(discriminant)
	intersection.position = add(start, muls(intersection.distance, direction))
	intersection.normal = normalize(sub(intersection.position, sphere.position))
	intersection.color = sphere.color
	return intersection

def findIntersection(start: Vec3d, direction: Vec3d, spheres: Sequence[Sphere]) -> Intersection:
	i1 = Intersection()
	for sphere in spheres:
		i2 = findSingleIntersection(start, direction, sphere)
		if i2.distance < i1.distance:
			i1 = i2
	return i1

def shadeSingleLight(intersection: Intersection, light: Light) -> Vec3d:
	geometry = max(-dot(light.direction, intersection.normal), 0.0)
	return muls(geometry, mul(intersection.color, light.color))

def shadeAtmosphere(intersection: Intersection, atmosphere_color: Vec3d) -> Vec3d:
	return muls(math.sqrt(intersection.position.z), atmosphere_color)

def shade(intersection: Intersection, world: World) -> Vec3d:
	if math.isinf(intersection.distance):
		return Vec3d(1, 1, 1)
	color = shadeAtmosphere(intersection, world.atmosphere_color)
	for light in world.lights:
		color = add(color, shadeSingleLight(intersection, light))
	return color

def colorU8fromF64(c: float) -> int:
	return int(min(255.0 * c, 255.0))

def writePixel(file, x: int, y: int, width: int, height: int, world: World) -> None:
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
    file.write("{} {} {} ".format(r, g, b))

def writeImage(file_path: str, world: World) -> None:
    WIDTH = 800
    HEIGHT = 600
    with open(file_path, 'w') as file:
        file.write("{}\n{}\n{}\n{}\n".format("P3", WIDTH, HEIGHT, 255))
        for y in range(HEIGHT):
            for x in range(WIDTH):
                writePixel(file, x, y, WIDTH, HEIGHT, world)

def main():
	print("Saving image")
	world = makeWorld()
	writeImage("image.ppm", world)

if __name__ == "__main__":
    main()
