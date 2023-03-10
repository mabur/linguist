interface Vec3d {
  x: number;
  y: number;
  z: number;
}

function vec3d(x: number, y: number, z: number): Vec3d {
  return { x: x, y: y, z: z };
}

function zero3d(): Vec3d {
  return { x: 0, y: 0, z: 0 };
}

function add(a: Vec3d, b: Vec3d): Vec3d {
  return vec3d(a.x + b.x, a.y + b.y, a.z + b.z);
}

function sub(a: Vec3d, b: Vec3d): Vec3d {
  return vec3d(a.x - b.x, a.y - b.y, a.z - b.z);
}

function mul(a: Vec3d, b: Vec3d): Vec3d {
  return vec3d(a.x * b.x, a.y * b.y, a.z * b.z);
}

function muls(a: number, b: Vec3d): Vec3d {
  return vec3d(a * b.x, a * b.y, a * b.z);
}

function dot(a: Vec3d, b: Vec3d): number {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

function squaredNorm(v: Vec3d): number {
  return dot(v, v);
}

function norm(v: Vec3d): number {
  return Math.sqrt(squaredNorm(v));
}

function normalize(v: Vec3d): Vec3d {
  return muls(1.0 / norm(v), v);
}

interface Sphere {
  position: Vec3d;
  squared_radius: number;
  color: Vec3d;
}

interface Light {
  direction: Vec3d;
  color: Vec3d;
}

interface World {
  spheres: Array<Sphere>;
  lights: Array<Light>;
  atmosphere_color: Vec3d;
}

interface Intersection {
  position: Vec3d;
  normal: Vec3d;
  distance: number;
  color: Vec3d;
}

function makeIntersection(): Intersection {
  return {
    position: zero3d(),
    normal: zero3d(),
    distance: Infinity,
    color: zero3d(),
  };
}

function makeWorld(): World {
  const R = 100000.0;
  const MAX_C = 1.0;
  const MIN_C = 0.1;
  const spheres = [
    {
      position: vec3d(-2., 0., 6.),
      squared_radius: 1.,
      color: vec3d(MAX_C, MAX_C, MIN_C),
    },
    {
      position: vec3d(0., 0., 5.),
      squared_radius: 1.,
      color: vec3d(MAX_C, MIN_C, MIN_C),
    },
    {
      position: vec3d(2., 0., 4.),
      squared_radius: 1.,
      color: vec3d(2.0 * MIN_C, 4.0 * MIN_C, MAX_C),
    },
    {
      position: vec3d(0., 1. + R, 0.),
      squared_radius: R * R,
      color: vec3d(MIN_C, MAX_C, MIN_C),
    },
    {
      position: vec3d(0., -1. - R, 0.),
      squared_radius: R * R,
      color: vec3d(MAX_C, MAX_C, MAX_C),
    },
  ];
  const lights = [
    { direction: vec3d(1., 1., 2.), color: muls(0.4, vec3d(1.0, 0.8, 0.5)) },
    { direction: vec3d(-1., -1., -2.), color: muls(0.4, vec3d(0.5, 0.5, 1.0)) },
  ];
  const atmosphere_color = muls(0.3, vec3d(0.5, 0.5, 1.0));
  return {
    spheres: spheres,
    lights: lights,
    atmosphere_color: atmosphere_color,
  };
}

function findSingleIntersection(
  start: Vec3d,
  direction: Vec3d,
  sphere: Sphere,
): Intersection {
  const intersection = makeIntersection();
  const offset = sub(sphere.position, start);
  const c = dot(direction, offset);
  if (c < 0.0) {
    return intersection;
  }
  const discriminant = c * c - squaredNorm(offset) + sphere.squared_radius;
  if (discriminant < 0.0) {
    return intersection;
  }
  intersection.distance = c - Math.sqrt(discriminant);
  intersection.position = add(start, muls(intersection.distance, direction));
  intersection.normal = normalize(sub(intersection.position, sphere.position));
  intersection.color = sphere.color;
  return intersection;
}

function findIntersection(
  start: Vec3d,
  direction: Vec3d,
  spheres: Array<Sphere>,
): Intersection {
  let i1 = makeIntersection();
  for (const sphere of spheres) {
    const i2 = findSingleIntersection(start, direction, sphere);
    if (i2.distance < i1.distance) {
      i1 = i2;
    }
  }
  return i1;
}

function shadeSingleLight(intersection: Intersection, light: Light): Vec3d {
  const geometry = Math.max(0.0, -dot(light.direction, intersection.normal));
  return muls(geometry, mul(intersection.color, light.color));
}

function shadeAtmosphere(
  intersection: Intersection,
  atmosphere_color: Vec3d,
): Vec3d {
  return muls(Math.sqrt(intersection.position.z), atmosphere_color);
}

function shade(intersection: Intersection, world: World): Vec3d {
  if (intersection.distance == Infinity) {
    return vec3d(1, 1, 1);
  }
  return world.lights.reduce(
    (color, light) => add(color, shadeSingleLight(intersection, light)),
    shadeAtmosphere(intersection, world.atmosphere_color),
  );
}

function colorU8fromF64(c: number): number {
  return Math.trunc(Math.min(255.0 * c, 255.0));
}

function serializePixel(
  x: number,
  y: number,
  width: number,
  height: number,
  world: World,
): string {
  const start = zero3d();
  const xd = x - width / 2;
  const yd = y - height / 2;
  const zd = height / 2;
  const direction = normalize(vec3d(xd, yd, zd));
  const intersection = findIntersection(start, direction, world.spheres);
  const color = shade(intersection, world);
  const r = colorU8fromF64(color.x);
  const g = colorU8fromF64(color.y);
  const b = colorU8fromF64(color.z);
  return `${r} ${g} ${b} `;
}

function serializeImage(world: World): string {
  const WIDTH = 800;
  const HEIGHT = 600;
  let s = `P3\n${WIDTH}\n${HEIGHT}\n255\n`;
  for (let y = 0; y < HEIGHT; y++) {
    for (let x = 0; x < WIDTH; x++) {
      s += serializePixel(x, y, WIDTH, HEIGHT, world);
    }
  }
  return s;
}

console.log("Saving image");
const world = makeWorld();
const image = serializeImage(world);
await Deno.writeTextFile("image.ppm", image);
