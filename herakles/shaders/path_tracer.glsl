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
 #include "scene.glsl"

bool triangleIntersect(const Ray ray, const Mesh mesh, const uint vertexID, out float dist) {
  return false;
}

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

/// Ray-scene intersection.
bool sceneIntersect(const Ray ray, out float t, out uint id, int skip) {
  float dist;
  t = INF;
  const uint numMeshes = Meshes.length();
  for (uint i = 0; i < numMeshes; ++i) {
    const Mesh mesh = Meshes[i];
    for (uint j = mesh.begin; j != mesh.end - 2; ++j) {
      if (j != skip && triangleIntersect(ray, mesh, j, dist)
          && dist - t < 0) {
        t = dist;
        id = j;
      }
    }
  }
  return t < INF;
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
  }

  return color;
}


#endif // !HERAKLES_SHADERS_PATH_TRACER_GLSL
