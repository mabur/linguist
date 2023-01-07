use std::fs::File;
use std::io::Result;
use std::io::prelude::*;

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
    squared_radius: f64,
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

fn closest_intersection(a: Intersection, b: Intersection) -> Intersection {
    if a.distance < b.distance {a} else {b}
}

fn make_spheres() -> Vec<Sphere> {
    let R = 100000.0;
    let MAX_C = 1.0;
    let MIN_C = 0.1;
    return vec![
        Sphere{position:[-2., 0., 6.], squared_radius:1., color: [MAX_C, MAX_C, MIN_C]},
        Sphere{position:[0., 0., 5.], squared_radius:1., color: [MAX_C, MIN_C, MIN_C]},
        Sphere{position:[2., 0., 4.], squared_radius:1., color: [2.0*MIN_C, 4.0*MIN_C, MAX_C]},
        Sphere{position:[0., 1.+R, 0.], squared_radius:R*R, color: [MIN_C, MAX_C, MIN_C]},
        Sphere{position:[0., -1.-R, 0.], squared_radius:R*R, color: [MAX_C, MAX_C, MAX_C]},
    ];
}

fn make_lights() -> Vec<Light> {
    vec![
        Light{direction:[1., 1., 2.], color:muls(0.4, &[1.0,0.8,0.5])},
        Light{direction:[-1., -1., -2.], color:muls(0.4, &[0.5,0.5,1.0])},
    ]
}

fn find_single_intersection(
    start: &Vec3d, direction: &Vec3d, sphere: &Sphere
) -> Intersection {
    let offset = sub(&sphere.position, start);
    let c = dot(direction, &offset);
    if c < 0.0 {
        return make_intersection()
    };
    let discriminant = c * c - squared_norm(&offset) + sphere.squared_radius;
    if discriminant < 0.0 {
        return make_intersection()
    } 
    let mut intersection = make_intersection();
    intersection.distance = c - discriminant.sqrt();
    intersection.position = add(&start, &muls(intersection.distance, &direction));
    intersection.normal = normalize(&sub(&intersection.position, &sphere.position));
    intersection.color = sphere.color;
    return intersection;
}

fn find_intersection(
    start: &Vec3d, direction: &Vec3d, spheres: &Vec<Sphere>
) -> Intersection{
    let mut intersection = make_intersection();
    for sphere in spheres.iter() {
        let i2 = find_single_intersection(start, direction, sphere);
        intersection = closest_intersection(intersection, i2);
    }
    return intersection;
}

fn shade_single_light(intersection: &Intersection, light: &Light) -> Vec3d{
    let geometry = 0.0_f64.max(-dot(&light.direction, &intersection.normal));
    return muls(geometry, &mul(&intersection.color, &light.color));
}

fn shade_atmosphere(intersection: &Intersection) -> Vec3d{
    return muls(intersection.position[2].sqrt() * 0.3, &[0.5, 0.5, 1.0]);
}

fn shade(intersection: &Intersection, lights: &Vec<Light>) -> Vec3d {
    if intersection.distance.is_infinite() {
        [1., 1., 1.];
    }
    let mut color = shade_atmosphere(intersection);
    for light in lights.iter() {
        color = add(&color, &shade_single_light(intersection, light));
    }
    return color;
}

fn color_u8_from_f64(c: f64) -> u8 {
    (255.0 * c).min(255.0) as u8
}

fn write_image(file_path: &str, spheres: &Vec<Sphere>, lights: &Vec<Light>) -> Result<()> {
    let mut file = File::create(file_path)?;
    let width = 800;
    let height = 600;
    let focal_length = height / 2;
    write!(file, "{}\n{}\n{}\n{}\n", "P3", width, height, 255)?;
    for y in 0..height {
        for x in 0..width {
            let start = [0., 0., 0.];
            let xd = (x - width / 2) as f64;
            let yd = (y - height / 2) as f64;
            let zd = (focal_length) as f64;
            let direction = normalize(&[xd, yd, zd]);
            let intersection = find_intersection(&start, &direction, &spheres);
            let color = shade(&intersection, &lights);
            let r = color_u8_from_f64(color[0]);
            let g = color_u8_from_f64(color[1]);
            let b = color_u8_from_f64(color[2]);
            write!(file, "{} {} {} ", r, g, b)?;
        }
    }
    return Ok(())
}

fn main() {
    println!("Saving image");
    let spheres = make_spheres();
    let lights = make_lights();
    write_image("image.ppm", &spheres, &lights);
}
