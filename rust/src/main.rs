struct Vec3d {
    x: f64,
    y: f64,
    z: f64,
}

fn add(a: &Vec3d, b: &Vec3d) -> Vec3d {
    Vec3d{x: a.x + b.x, y: a.y + b.y, z: a.z + b.z}
}

fn sub(a: &Vec3d, b: &Vec3d) -> Vec3d {
    Vec3d{x: a.x - b.x, y: a.y - b.y, z: a.z - b.z}
}

fn mul(a: &Vec3d, b: &Vec3d) -> Vec3d {
    Vec3d{x: a.x * b.x, y: a.y * b.y, z: a.z * b.z}
}

fn muls(a: f64, b: &Vec3d) -> Vec3d {
    Vec3d{x: a * b.x, y: a * b.y, z: a * b.z}
}

fn dot(a: &Vec3d, b: &Vec3d) -> f64 {
    a.x * b.x + a.y * b.y + a.z * b.z
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
        position: Vec3d{x: 0.0, y: 0.0, z: 0.0},
        normal: Vec3d{x: 0.0, y: 0.0, z: 0.0},
        distance: f64::INFINITY,
        color: Vec3d{x: 0.0, y: 0.0, z: 0.0},
    }
}

fn makeSpheres() -> Vec<Sphere> {
    let R = 100000.0;
    let MAX_C = 1.0;
    let MIN_C = 0.1;
    return vec![
        Sphere{position: Vec3d{x: -2.0, y: 0.0, z: 6.0}, squaredRadius: 1.0, color: Vec3d{x: MAX_C, y: MAX_C, z: MIN_C}},
        Sphere{position: Vec3d{x: 0.0, y: 0.0, z: 5.0}, squaredRadius: 1.0, color: Vec3d{x: MAX_C, y: MIN_C, z: MIN_C}},
        Sphere{position: Vec3d{x: 2.0, y: 0.0, z: 4.0}, squaredRadius: 1.0, color: Vec3d{x: 2.0*MIN_C, y: 4.0*MIN_C, z: MAX_C}},
        Sphere{position: Vec3d{x: 0.0, y: 1.0+R, z: 0.0}, squaredRadius: R * R, color: Vec3d{x: MIN_C, y: MAX_C, z: MIN_C}},
        Sphere{position: Vec3d{x: 0.0, y: -1.0 - R, z: 0.0}, squaredRadius: R * R, color: Vec3d{x: MAX_C, y: MAX_C, z: MAX_C}},
    ];
}

fn main() {
    println!("Hello, world!");
}
