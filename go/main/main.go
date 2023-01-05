package main

import (
	"fmt"
	"math"
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
	distanc  float64
	color    Vec3d
}

func main() {
	fmt.Println("Ray tracing")
}
