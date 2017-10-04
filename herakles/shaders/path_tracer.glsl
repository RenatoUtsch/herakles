/*
 * Copyright 2017 Renato Utsch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Path tracer implementation.
 */

 #ifndef HERAKLES_SHADERS_PATH_TRACER_GLSL
 #define HERAKLES_SHADERS_PATH_TRACER_GLSL

 #include "random.glsl"

/**
 * Represents a ray travelling through the scene.
 */
struct Ray {
  vec3 origin;
  vec3 direction;
};

/**
 * Pinhole camera used in the scene.
 */
struct PinholeCamera {
  vec3 position;
  float fov;
  vec3 direction;
  vec3 up;
  vec3 right;
};

/**
 * Represents a single area light.
 */
struct AreaLight {
  vec3 emission;
  uint meshID;
};

/**
 * Represents a single mesh instance.
 */
struct Mesh {
  uint begin;
  uint end;
  uint materialID;
  //uint transformID;
};

/**
 * Represents a single mesh's material. 
 */
struct Material {
  /// Reflectivity of the diffuse surface.
  vec3 kd;
};

/* layout(binding = 4, std430) buffer MeshesBuffer { */
/*   Mesh Meshes[]; */
/* }; */
/* layout(binding = 5, std430) buffer IndicesBuffer { */
/*   uint Indices[]; */
/* }; */
/* layout(binding = 6, std430) buffer VerticesBuffer { */
/*   vec3 Vertices[]; */
/* }; */
/* layout(binding = 7, std430) buffer MaterialsBuffer { */
/*   Material Materials[]; */
/* }; */
/* layout(binding = 8, std430) buffer TransformsBuffer { */
/*   mat4 Transforms[]; */
/* }; */

/* bool triangleIntersect(const Ray ray, const Mesh mesh, const uint vertexID, out float dist) { */
/*   return false; */
/* } */

/* /// Ray-scene intersection. */
/* bool sceneIntersect(const Ray ray, out float t, out uint id, int skip) { */
/*   float dist; */
/*   t = INF; */
/*   const uint numMeshes = Meshes.length(); */
/*   for (uint i = 0; i < numMeshes; ++i) { */
/*     const Mesh mesh = Meshes[i]; */
/*     for (uint j = mesh.begin; j != mesh.end - 2; ++j) { */
/*       if (j != skip && triangleIntersect(ray, mesh, j, dist) */
/*           && dist - t < 0) { */
/*         t = dist; */
/*         id = j; */
/*       } */
/*     } */
/*   } */
/*   return t < INF; */
/* } */

struct Sphere {
  float radius;
  vec3 position;
  vec3 emission;
  vec3 color;
  uint material; // DIFF, SPEC or REFR.
};


const uint NUM_SAMPLES = 50;
const uint MAX_DEPTH = 4;

const float EPSILON = 1e-7;
const float INF = 1e20;
const float M_PI = 3.1415926535;

const uint DIFF = 0;
const uint SPEC = 1;
const uint REFR = 2;

const uint NUM_SPHERES = 9;
const Sphere gSpheres[NUM_SPHERES] = Sphere[](
  Sphere(1e5, vec3( 1e5+1,40.8,81.6), vec3(.0),vec3(.75,.25,.25),DIFF),//Left
  Sphere(1e5, vec3(-1e5+99,40.8,81.6),vec3(.0),vec3(.25,.25,.75),DIFF),//Right
  Sphere(1e5, vec3(50,40.8, 1e5),     vec3(.0),vec3(.75),        DIFF),//Back
  Sphere(1e5, vec3(50,40.8,-1e5+170), vec3(.0),vec3(.0),         DIFF),//Front
  Sphere(1e5, vec3(50, 1e5, 81.6),    vec3(.0),vec3(.75),        DIFF),//Bottom
  Sphere(1e5, vec3(50,-1e5+81.6,81.6),vec3(.0),vec3(.75),        DIFF),//Top
  Sphere(16.5,vec3(27,16.5,47),       vec3(.0),vec3(.999),       SPEC),//Mirror
  Sphere(16.5,vec3(73,16.5,78),       vec3(.0),vec3(.999),       REFR),//Glass
  Sphere(600, vec3(50,681.6-.27,81.6),vec3(12),vec3(.0),         DIFF) //Light
);

/// Ray-sphere intersection. Returns distance and whether hits.
bool sphereIntersect(const Ray ray, const Sphere sphere, out float dist) {
  const vec3 op = sphere.position - ray.origin;
  const float b = dot(op, ray.direction);
  float det = b * b - dot(op, op) + sphere.radius * sphere.radius;
  if (det <= -EPSILON) return false;
  det = sqrt(det);
  const float t1 = b - det;
  const float t2 = b + det;
  if (t1 > EPSILON) {
    dist = t1;
    return true;
  } else if (t2 > EPSILON) {
    dist = t2;
    return true;
  }
  return false;
}

/// Ray-scene intersection.
bool sceneIntersect(const Ray ray, out float t, out int id, int skip) {
  float dist;
  t = INF;
  for (int i = 0; i < NUM_SPHERES; ++i) {
    if (i != skip && sphereIntersect(ray, gSpheres[i], dist)
        && dist - t <= -EPSILON) {
      t = dist;
      id = i;
    }
  }
  return t <= INF - EPSILON;
}

/// Returns estimated radiance along ray.
vec3 radiance(Ray ray) {
  float t;
  int id = -1;
  vec3 color = vec3(0.0f);
  vec3 reflectance = vec3(1.0f);
  for (int depth = 0; depth < MAX_DEPTH; ++depth) {
    if (!sceneIntersect(ray, t, id, id)) break;
    Sphere sphere = gSpheres[id];
    const vec3 intersection = ray.origin + ray.direction * t;
    const vec3 normal = normalize(intersection - sphere.position);
    const vec3 orientedNormal = dot(normal, ray.direction) <= -EPSILON
                                ? normal : normal * -1.0f;

    color += reflectance * sphere.emission;
    reflectance *= sphere.color;

    if (sphere.material == DIFF) { // Ideal diffuse reflection
      const float u1 = rand();
      const float u2 = rand();
      const float theta = 2.0f * M_PI * u1;
      const float phi = sqrt(u2);

      const vec3 w = orientedNormal;
      const vec3 u = normalize(
          cross(abs(w.x) >= 0.1f + EPSILON ? vec3(0, 1, 0) : vec3(1, 0, 0), w));
      const vec3 v = cross(w, u);

      const vec3 direction = normalize(
          u * cos(theta) * phi +
          v * sin(theta) * phi +
          w * sqrt(1.0f - u2));

      ray = Ray(intersection, direction);
    } else if (sphere.material == SPEC) { // Ideal specular reflection
      ray = Ray(intersection, reflect(ray.direction, normal));
    } else { // Ideal dielectric reflection/refraction
      const Ray reflectionRay = Ray(intersection, reflect(ray.direction, normal));
      const bool fromOutsideToInside = dot(normal, orientedNormal) >= EPSILON;
      const float nc = 1.0f, nt = 1.5f;
      const float nnt = fromOutsideToInside ? nc / nt : nt / nc;
      const float ddn = dot(ray.direction, orientedNormal);
      const float cos2t = 1.0f - nnt * nnt * (1.0f - ddn * ddn);

      if (cos2t <= -EPSILON) { // Total internal reflection
        ray = reflectionRay;
      } else { // Choose reflection or refraction
        const vec3 tdir = normalize(ray.direction * nnt - normal * (
            ((fromOutsideToInside ? 1.0f : -1.0f) * (ddn * nnt + sqrt(cos2t)))));
        const float a = nt - nc;
        const float b = nt + nc;
        const float r0 = a * a / (b * b);
        const float c = 1.0f - (fromOutsideToInside ? -ddn : dot(tdir, normal));
        const float re = r0 + (1.0f - r0) * c * c * c * c * c;
        const float tr = 1.0f - re;
        const float p = 0.25f + 0.5f * re;
        const float rp = re / p;
        const float tp = tr / (1.0f - p);

        if (rand() - p <= -EPSILON) { // Russian roulette
          reflectance *= rp;
          ray = reflectionRay;
        } else {
          reflectance *= tp;
          ray = Ray(intersection, tdir);
        }
      }
    }
  }

  return color;
}


#endif // HERAKLES_SHADERS_PATH_TRACER_GLSL
