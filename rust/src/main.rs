
type Vec3d = [f64; 3];

fn add(a: &Vec3d, b: &Vec3d) -> Vec3d {
    [a[0] + b[0], a[1] + b[1], a[2] + b[2]]
}

fn sub(a: &Vec3d, b: &Vec3d) -> Vec3d {
    [a[0] - b[0], a[1] - b[1], a[2] - b[2]]
}

fn mul(a: &Vec3d, b: &Vec3d) -> Vec3d {
    [a[0] * b[0], a[1] * b[1], a[2] * b[2]]
}

fn muls(a: f64, b: &Vec3d) -> Vec3d {
    [a * b[0], a * b[1], a * b[2]]
}

fn dot(a: &Vec3d, b: &Vec3d) -> f64 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

fn squared_norm(v: &Vec3d) -> f64 {
    dot(v, v)
}

fn norm(v: &Vec3d) -> f64 {
    squared_norm(v).sqrt()
}

fn normalize(v: &Vec3d) -> Vec3d {
    muls(1.0 / norm(v), v)
}

struct Sphere {
    position: Vec3d,
    squaredRadius: f64,
    color: Vec3d,
}

struct Light {
    direction: Vec3d,
    color: Vec3d,
}

struct Intersection {
    position: Vec3d,
    normal: Vec3d,
    distance: f64,
    color: Vec3d,
}

fn make_intersection() -> Intersection {
    Intersection{
        position: [0.0, 0.0, 0.0],
        normal: [0.0, 0.0, 0.0],
        distance: f64::INFINITY,
        color: [0.0, 0.0, 0.0],
    }
}

fn makeSpheres() -> Vec<Sphere> {
    let R = 100000.0;
    let MAX_C = 1.0;
    let MIN_C = 0.1;
    return vec![
        Sphere{position:[-2., 0., 6.], squaredRadius:1., color: [MAX_C, MAX_C, MIN_C]},
        Sphere{position:[0., 0., 5.], squaredRadius:1., color: [MAX_C, MIN_C, MIN_C]},
        Sphere{position:[2., 0., 4.], squaredRadius:1., color: [2.0*MIN_C, 4.0*MIN_C, MAX_C]},
        Sphere{position:[0., 1.+R, 0.], squaredRadius:R*R, color: [MIN_C, MAX_C, MIN_C]},
        Sphere{position:[0., -1.-R, 0.], squaredRadius:R*R, color: [MAX_C, MAX_C, MAX_C]},
    ];
}

fn main() {
    println!("Hello, world!");
}
