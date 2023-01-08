
class Vec3d {
    x: number = 0;
    y: number = 0;
    z: number = 0;
}

function vec3d(x: number, y: number, z: number) : Vec3d {
    return {x: x, y:y, z: z}
}

function add(a: Vec3d, b: Vec3d) : Vec3d {
    return vec3d(a.x + b.x, a.y + b.y, a.z + b.z)
}

function sub(a: Vec3d, b: Vec3d) : Vec3d {
    return vec3d(a.x - b.x, a.y - b.y, a.z - b.z)
}

function mul(a: Vec3d, b: Vec3d) : Vec3d {
    return vec3d(a.x * b.x, a.y * b.y, a.z * b.z)
}

function muls(a: number, b: Vec3d) : Vec3d {
    return vec3d(a * b.x, a * b.y, a * b.z)
}

function dot(a: Vec3d, b: Vec3d) : number {
    return a.x * b.x + a.y * b.y + a.z * b.z
}

function squared_norm(v: Vec3d) : number {
    return dot(v, v)
}

function norm(v: Vec3d) : number {
    return Math.sqrt(squared_norm(v))
}

function normalize(v: Vec3d) : Vec3d {
    return muls((1.0 / norm(v)), v)
}

class Sphere {
    position: Vec3d = new Vec3d();
    squared_radius: number = 0;
    color: Vec3d = new Vec3d();
}

class Light {
    direction: Vec3d = new Vec3d();
    color: Vec3d = new Vec3d();
}

class World {
    spheres: Array<Sphere> = [];
    lights: Array<Light> = [];
    atmosphere_color: Vec3d =  new Vec3d();
}

class Intersection {
    position: Vec3d = new Vec3d();
    normal: Vec3d = new Vec3d();
    distance: number = Infinity;
    color: Vec3d = new Vec3d();
}


function make_world() : World {
    const R = 100000.0;
    const MAX_C = 1.0;
    const MIN_C = 0.1;
    const spheres = [
        {position: vec3d(-2., 0., 6.), squared_radius: 1., color: vec3d(MAX_C, MAX_C, MIN_C)},
        {position: vec3d(0., 0., 5.), squared_radius: 1., color: vec3d(MAX_C, MIN_C, MIN_C)},
        {position: vec3d(2., 0., 4.), squared_radius: 1., color: vec3d(2.0*MIN_C, 4.0*MIN_C, MAX_C)},
        {position: vec3d(0., 1.+R, 0.), squared_radius: R*R, color: vec3d(MIN_C, MAX_C, MIN_C)},
        {position: vec3d(0., -1.-R, 0.), squared_radius: R*R, color: vec3d(MAX_C, MAX_C, MAX_C)},
    ];
    const lights = [
        {direction:vec3d(1., 1., 2.), color: muls(0.4, vec3d(1.0,0.8,0.5))},
        {direction:vec3d(-1., -1., -2.), color: muls(0.4, vec3d(0.5,0.5,1.0))},
    ];
    const atmosphere_color = muls(0.3, vec3d(0.5, 0.5, 1.0));
    return {spheres:spheres, lights:lights, atmosphere_color:atmosphere_color};
}

function find_single_intersection(
    start: Vec3d, direction: Vec3d, sphere: Sphere
) : Intersection {
    let intersection = new Intersection();
    const offset = sub(sphere.position, start);
    const c = dot(direction, offset);
    if (c < 0.0) {
        return intersection
    };
    const discriminant = c * c - squared_norm(offset) + sphere.squared_radius;
    if (discriminant < 0.0) {
        return intersection
    } 
    intersection.distance = c - Math.sqrt(discriminant);
    intersection.position = add(start, muls(intersection.distance, direction));
    intersection.normal = normalize(sub(intersection.position, sphere.position));
    intersection.color = sphere.color;
    return intersection;
}

function find_intersection(
    start: Vec3d, direction: Vec3d, spheres: Array<Sphere>
) : Intersection {
    let i1 = new Intersection();
    for (const sphere of spheres) {
        const i2 = find_single_intersection(start, direction, sphere);
        if (i2.distance < i1.distance) {
            i1 = i2
        }
    }
    return i1;
}

function shade_single_light(intersection: Intersection, light: Light) : Vec3d{
    let geometry = Math.max(0.0, -dot(light.direction, intersection.normal));
    return muls(geometry, mul(intersection.color, light.color))
}

function shade_atmosphere(intersection: Intersection, atmosphere_color: Vec3d) : Vec3d{
    return muls(Math.sqrt(intersection.position.z), atmosphere_color)
}

function shade(intersection: Intersection, world: World) : Vec3d {
    if (intersection.distance == Infinity) {
        return vec3d(1, 1, 1);
    }
    let color = shade_atmosphere(intersection, world.atmosphere_color);
    for (const light of world.lights) {
        color = add(color, shade_single_light(intersection, light));
    }
    return color;
}

function color_u8_from_number(c: number) : number {
    return Math.trunc(Math.min(255.0 * c, 255.0))
}

function serialize_pixel(
    x: number,
    y: number,
    width: number,
    height: number,
    world: World,
) : string {
    const start = new Vec3d();
    const xd = x - width / 2;
    const yd = y - height / 2;
    const zd = height / 2;
    const direction = normalize(vec3d(xd, yd, zd));
    const intersection = find_intersection(start, direction, world.spheres);
    const color = shade(intersection, world);
    const r = color_u8_from_number(color.x);
    const g = color_u8_from_number(color.y);
    const b = color_u8_from_number(color.z);
    return r.toString() + " " + g.toString() + " " + b.toString() + " "
}

function serialize_image(world: World) : string {
    const WIDTH = 800;
    const HEIGHT = 600;
    let s = "P3\n" + WIDTH.toString() + "\n" + HEIGHT.toString() + "\n255\n"
    for (let y = 0; y < HEIGHT; y++) {
        for (let x = 0; x < WIDTH; x++) {
            s += serialize_pixel(x, y, WIDTH, HEIGHT, world)
        }
    }
    return s   
}

console.log("Saving image")
const world = make_world();
const image = serialize_image(world);
await Deno.writeTextFile("image.ppm", image);
console.log("Done")