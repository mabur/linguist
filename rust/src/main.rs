use std::fs::File;
use std::io::prelude::*;
use std::io::BufWriter;
use std::ops;

#[derive(Copy, Clone, Default)]
struct Vec3d(f64, f64, f64);

impl Vec3d {
    fn squared_norm(self) -> f64 {
        self.dot(self)
    }

    fn norm(self) -> f64 {
        self.squared_norm().sqrt()
    }

    fn normalize(self) -> Vec3d {
        (1.0 / self.norm()) * self
    }

    fn dot(self, other: Vec3d) -> f64 {
        self.0 * other.0 + self.1 * other.1 + self.2 * other.2
    }
}

impl ops::Add<Vec3d> for Vec3d {
    type Output = Vec3d;
    fn add(self, right: Vec3d) -> Vec3d {
        Vec3d(self.0 + right.0, self.1 + right.1, self.2 + right.2)
    }
}

impl ops::Sub<Vec3d> for Vec3d {
    type Output = Vec3d;
    fn sub(self, right: Vec3d) -> Vec3d {
        Vec3d(self.0 - right.0, self.1 - right.1, self.2 - right.2)
    }
}

impl ops::Mul<Vec3d> for Vec3d {
    type Output = Vec3d;
    fn mul(self, right: Vec3d) -> Vec3d {
        Vec3d(self.0 * right.0, self.1 * right.1, self.2 * right.2)
    }
}

impl ops::Mul<Vec3d> for f64 {
    type Output = Vec3d;
    fn mul(self, right: Vec3d) -> Vec3d {
        Vec3d(self * right.0, self * right.1, self * right.2)
    }
}

#[derive(Copy, Clone)]
struct Sphere {
    position: Vec3d,
    squared_radius: f64,
    color: Vec3d,
}

#[derive(Copy, Clone)]
struct Light {
    direction: Vec3d,
    color: Vec3d,
}

struct World {
    spheres: Vec<Sphere>,
    lights: Vec<Light>,
    atmosphere_color: Vec3d,
}

#[derive(Copy, Clone)]
struct Intersection {
    position: Vec3d,
    normal: Vec3d,
    distance: f64,
    color: Vec3d,
}

impl Default for Intersection {
    fn default() -> Self {
        Intersection {
            position: Default::default(),
            normal: Default::default(),
            distance: f64::INFINITY,
            color: Default::default(),
        }
    }
}

fn make_world() -> World {
    const R: f64 = 100000.0;
    const MAX_C: f64 = 1.0;
    const MIN_C: f64 = 0.1;
    let spheres = vec![
        Sphere {
            position: Vec3d(-2., 0., 6.),
            squared_radius: 1.,
            color: Vec3d(MAX_C, MAX_C, MIN_C),
        },
        Sphere {
            position: Vec3d(0., 0., 5.),
            squared_radius: 1.,
            color: Vec3d(MAX_C, MIN_C, MIN_C),
        },
        Sphere {
            position: Vec3d(2., 0., 4.),
            squared_radius: 1.,
            color: Vec3d(2.0 * MIN_C, 4.0 * MIN_C, MAX_C),
        },
        Sphere {
            position: Vec3d(0., 1. + R, 0.),
            squared_radius: R * R,
            color: Vec3d(MIN_C, MAX_C, MIN_C),
        },
        Sphere {
            position: Vec3d(0., -1. - R, 0.),
            squared_radius: R * R,
            color: Vec3d(MAX_C, MAX_C, MAX_C),
        },
    ];
    let lights = vec![
        Light {
            direction: Vec3d(1., 1., 2.),
            color: 0.4 * Vec3d(1.0, 0.8, 0.5),
        },
        Light {
            direction: Vec3d(-1., -1., -2.),
            color: 0.4 * Vec3d(0.5, 0.5, 1.0),
        },
    ];
    let atmosphere_color = 0.3 * Vec3d(0.5, 0.5, 1.0);
    World {
        spheres,
        lights,
        atmosphere_color,
    }
}

fn find_single_intersection(start: Vec3d, direction: Vec3d, sphere: Sphere) -> Intersection {
    let mut intersection = Default::default();
    let offset = sphere.position - start;
    let c = direction.dot(offset);
    if c < 0.0 {
        return intersection;
    };
    let discriminant = c * c - offset.squared_norm() + sphere.squared_radius;
    if discriminant < 0.0 {
        return intersection;
    }
    intersection.distance = c - discriminant.sqrt();
    intersection.position = start + intersection.distance * direction;
    intersection.normal = (intersection.position - sphere.position).normalize();
    intersection.color = sphere.color;
    intersection
}

fn find_intersection(start: Vec3d, direction: Vec3d, spheres: &[Sphere]) -> Intersection {
    spheres
        .iter()
        .map(|sphere| find_single_intersection(start, direction, *sphere))
        .min_by(|a, b| a.distance.partial_cmp(&b.distance).unwrap())
        .unwrap()
}

fn shade_single_light(intersection: Intersection, light: Light) -> Vec3d {
    let geometry = 0.0_f64.max(-(light.direction.dot(intersection.normal)));
    geometry * (intersection.color * light.color)
}

fn shade_atmosphere(intersection: Intersection, atmosphere_color: Vec3d) -> Vec3d {
    intersection.position.2.sqrt() * atmosphere_color
}

fn shade(intersection: Intersection, world: &World) -> Vec3d {
    if intersection.distance.is_infinite() {
        return Vec3d(1., 1., 1.);
    }
    let color = shade_atmosphere(intersection, world.atmosphere_color);
    return world
        .lights
        .iter()
        .map(|light| shade_single_light(intersection, *light))
        .fold(color, |a, b| a + b);
}

fn color_u8_from_f64(c: f64) -> u8 {
    (255.0 * c).min(255.0) as u8
}

fn write_pixel(
    writer: &mut BufWriter<File>,
    x: i32,
    y: i32,
    width: i32,
    height: i32,
    world: &World,
) {
    let start = Vec3d(0.0, 0.0, 0.0);
    let xd = (x - width / 2) as f64;
    let yd = (y - height / 2) as f64;
    let zd = (height / 2) as f64;
    let direction = Vec3d(xd, yd, zd).normalize();
    let intersection = find_intersection(start, direction, &world.spheres);
    let color = shade(intersection, world);
    let r = color_u8_from_f64(color.0);
    let g = color_u8_from_f64(color.1);
    let b = color_u8_from_f64(color.2);
    write!(writer, "{} {} {} ", r, g, b).unwrap();
}

fn write_image(file_path: &str, world: &World) {
    let file = File::create(file_path).unwrap();
    let mut writer = BufWriter::new(file);
    const WIDTH: i32 = 800;
    const HEIGHT: i32 = 600;
    write!(writer, "P3\n{}\n{}\n{}\n", WIDTH, HEIGHT, 255).unwrap();
    for y in 0..HEIGHT {
        for x in 0..WIDTH {
            write_pixel(&mut writer, x, y, WIDTH, HEIGHT, world);
        }
    }
}

fn main() {
    println!("Saving image");
    let world = make_world();
    write_image("image.ppm", &world);
}
