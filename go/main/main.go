package main

import (
	"fmt"
	"math"
	"os"
)

type Vec3d struct {
	x float64
	y float64
	z float64
}

func add(a, b Vec3d) Vec3d {
	return Vec3d{a.x + b.x, a.y + b.y, a.z + b.z}
}

func sub(a, b Vec3d) Vec3d {
	return Vec3d{a.x - b.x, a.y - b.y, a.z - b.z}
}

func mul(a, b Vec3d) Vec3d {
	return Vec3d{a.x * b.x, a.y * b.y, a.z * b.z}
}

func muls(a float64, b Vec3d) Vec3d {
	return Vec3d{a * b.x, a * b.y, a * b.z}
}

func dot(a, b Vec3d) float64 {
	return a.x*b.x + a.y*b.y + a.z*b.z
}

func squaredNorm(v Vec3d) float64 {
	return dot(v, v)
}

func norm(v Vec3d) float64 {
	return math.Sqrt(squaredNorm(v))
}

func normalize(v Vec3d) Vec3d {
	return muls(1.0/norm(v), v)
}

type Sphere struct {
	position      Vec3d
	squaredRadius float64
	color         Vec3d
}

type Light struct {
	direction Vec3d
	color     Vec3d
}

type Intersection struct {
	position Vec3d
	normal   Vec3d
	distance float64
	color    Vec3d
}

func makeIntersection() Intersection {
	return Intersection{distance: math.Inf(1)}
}

func closestIntersection(a, b Intersection) Intersection {
	if a.distance < b.distance {
		return a
	}
	return b
}

func makeSpheres() []Sphere {
	R := 100000.0
	MAX_C := 1.0
	MIN_C := 0.1
	return []Sphere{
		{Vec3d{-2, 0, 6}, 1, Vec3d{MAX_C, MAX_C, MIN_C}},
		{Vec3d{0, 0, 5}, 1, Vec3d{MAX_C, MIN_C, MIN_C}},
		{Vec3d{2, 0, 4}, 1, Vec3d{2 * MIN_C, 4 * MIN_C, MAX_C}},
		{Vec3d{0, 1 + R, 0}, R * R, Vec3d{MIN_C, MAX_C, MIN_C}},
		{Vec3d{0, -1 - R, 0}, R * R, Vec3d{MAX_C, MAX_C, MAX_C}},
	}
}

func makeLights() []Light {
	return []Light{
		{Vec3d{+1, +1, +2}, muls(0.4, Vec3d{1, 0.8, 0.5})},
		{Vec3d{-1, -1, -2}, muls(0.4, Vec3d{0.5, 0.5, 1})},
	}
}

func findSingleIntersection(start Vec3d, direction Vec3d, sphere Sphere) Intersection {
	intersection := makeIntersection()
	offset := sub(sphere.position, start)
	c := dot(direction, offset)
	if c < 0.0 {
		return intersection
	}
	discriminant := c*c - squaredNorm(offset) + sphere.squaredRadius
	if discriminant < 0.0 {
		return intersection
	}
	intersection.distance = c - math.Sqrt(discriminant)
	intersection.position = add(start, muls(intersection.distance, direction))
	intersection.normal = normalize(sub(intersection.position, sphere.position))
	intersection.color = sphere.color
	return intersection
}

func fold[Source, Target any](collection []Source, f func(Source, Target) Target, init Target) Target {
	result := init
	for _, x := range collection {
		result = f(x, result)
	}
	return result
}

func findIntersection(start Vec3d, direction Vec3d, spheres []Sphere) Intersection {
	intersection := Intersection{}
	distance := math.Inf(+1)
	for _, sphere := range spheres {
		i := findSingleIntersection(start, direction, sphere)
		if i.distance < distance {
			intersection = i
			distance = i.distance
		}
	}
	return intersection
}

func findIntersectionFold(start Vec3d, direction Vec3d, spheres []Sphere) Intersection {
	intersection := makeIntersection()
	closestSphereIntersection := func(s Sphere, i1 Intersection) Intersection {
		i2 := findSingleIntersection(start, direction, s)
		return closestIntersection(i1, i2)
	}
	return fold(spheres, closestSphereIntersection, intersection)
}

func shadeSingleLight(intersection Intersection, light Light) Vec3d {
	geometry := math.Max(-dot(light.direction, intersection.normal), 0.0)
	return muls(geometry, mul(intersection.color, light.color))
}

func shadeAtmosphere(intersection Intersection) Vec3d {
	return muls(math.Sqrt(intersection.position.z)*0.3, Vec3d{0.5, 0.5, 1})
}

func shade(intersection Intersection, lights []Light) Vec3d {
	if math.IsInf(intersection.distance, 1) {
		return Vec3d{1, 1, 1}
	}
	color := shadeAtmosphere(intersection)
	for _, light := range lights {
		color = add(color, shadeSingleLight(intersection, light))
	}
	return color
}

func shadeFold(intersection Intersection, lights []Light) Vec3d {
	if math.IsInf(intersection.distance, 1) {
		return Vec3d{1, 1, 1}
	}
	color := shadeAtmosphere(intersection)
	addLight := func(light Light, c Vec3d) Vec3d {
		return add(shadeSingleLight(intersection, light), c)
	}
	return fold(lights, addLight, color)
}

func colorU8fromF64(c float64) uint8 {
	return uint8(math.Min(255.0*c, 255.0))
}

func writeImage(file_path string, spheres []Sphere, lights []Light) {
	file, _ := os.Create(file_path)
	defer file.Close()
	width := 800
	height := 600
	focal_length := height / 2
	fmt.Fprintf(file, "P3\n%d\n%d\n%d\n", width, height, 255)
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			start := Vec3d{0, 0, 0}
			xd := float64(x - width/2)
			yd := float64(y - height/2)
			zd := float64(focal_length)
			direction := normalize(Vec3d{xd, yd, zd})
			intersection := findIntersection(start, direction, spheres)
			color := shade(intersection, lights)
			r := colorU8fromF64(color.x)
			g := colorU8fromF64(color.y)
			b := colorU8fromF64(color.z)
			fmt.Fprintf(file, "%d %d %d ", r, g, b)
		}
	}
}

func main() {
	fmt.Println("Saving image")
	spheres := makeSpheres()
	lights := makeLights()
	writeImage("image.ppm", spheres, lights)
}
