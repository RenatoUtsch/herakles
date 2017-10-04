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

const uint NUM_SAMPLES = 10;
const uint MAX_DEPTH = 4;

const float EPSILON = 1e-7;
const float INF = 1e20;
const float M_PI = 3.14159265359f;

/// Returns the (u, v) barycentric coordinates of an uniform triangle sample.
/// PBRTv3 page 781.
vec2 uniformTriangleSample() {
  const float u0 = rand();
  const float u1 = rand();
  const float su0 = sqrt(u0);
  return vec2(1 - su0, u1 * su0);
}

/// Uniformly samples a point in a triangle.
/// Returns an interaction representing this sampled point.
Interaction sampleTriangle(const uint meshID, const uint begin) {
  const vec2 b = uniformTriangleSample();
  const float b2 = 1.0f - b[0] - b[1];
  const vec3 v0 = Vertices[Indices[begin]];
  const vec3 v1 = Vertices[Indices[begin + 1]];
  const vec3 v2 = Vertices[Indices[begin + 2]];
  const vec3 n0 = Normals[Indices[begin]];
  const vec3 n1 = Normals[Indices[begin + 1]];
  const vec3 n2 = Normals[Indices[begin + 2]];
  const vec3 point = b[0] * v0 + b[1] * v1 + b2 * v2;
  const vec3 normal = b[0] * n0 + b[1] * n1 + b2 * n2;

  return Interaction(point, meshID, normal, false);
}

/// Uniformly samples one area light source. The area light source is chosen
/// uniformly, excluding the first, unused area light.
/// Returns the light contribution from the intersection point. This may be 0
/// if the area light is not reachable from the intersection point.
/// Be sure the number of area lights is > 1 when calling this.
vec3 sampleLight(const Interaction isect) {
  const AreaLight light = AreaLights[urand(1, NUM_AREA_LIGHTS)];
  const Mesh mesh = Meshes[light.meshID];

  // Chooses a triangle from the mesh at random.
  // TODO(renatoutsch): maybe take the area of the triangles into account.
  uint begin = mesh.begin + 3 * urand((mesh.end - mesh.begin) / 3);


  // TODO
  return vec3(0.0f);
}




/// Triangle intersection.
/// Normal is not normalized and not oriented.
bool intersectsTriangle(const Ray ray, const uint begin, out float t,
                       out vec3 normal) {
  const vec3 v0 = Vertices[Indices[begin]];
  const vec3 v0v1 = Vertices[Indices[begin + 1]] - v0;
  const vec3 v0v2 = Vertices[Indices[begin + 2]] - v0;
  const vec3 pvec = cross(ray.direction, v0v2);
  const float det = dot(v0v1, pvec);

  // Ray and triangle are parallel if det is close to 0.
  if (abs(det) < EPSILON) return false;

  const float invDet = 1.0f / det;
  const vec3 tvec = ray.origin - v0;
  const float u = dot(tvec, pvec) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false;
  }

  const vec3 qvec = cross(tvec, v0v1);
  const float v = dot(ray.direction, qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }

  t = dot(v0v2, qvec) * invDet;
  normal = Normals[Indices[begin]] * u
         + Normals[Indices[begin + 1]] * v
         + Normals[Indices[begin + 2]] * (1.0f - u - v);
  return true;
}

/// Ray-scene intersection.
bool intersectsScene(const Ray ray, out Interaction isect) {
  bool intersected = false;
  float t = INF;
  uint meshID;
  vec3 normal;

  float currT;
  vec3 currNormal;
  for (uint i = 0; i < NUM_MESHES; ++i) {
    const Mesh mesh = Meshes[i];
    for (uint j = mesh.begin; j < mesh.end; j += 3) {
      if (intersectsTriangle(ray, j, currT, currNormal) &&
          currT < t  && currT > EPSILON) {
        intersected = true;
        t = currT;
        meshID = i;
        normal = currNormal;
      }
    }
  }

  if (!intersected) {
    return false;
  }

  normal = normalize(normal);
  const bool backface = dot(normal, ray.direction) <= -EPSILON ? false : true;

  isect = Interaction(
      ray.origin + ray.direction * t,
      meshID,
      backface ? normal * -1.0f : normal,
      backface);

  return true;
}

/// Returns estimated radiance along ray.
vec3 radiance(Ray ray) {
  vec3 color = vec3(0.0f);
  vec3 beta = vec3(1.0f);
  Interaction isect;
  for (int depth = 0; depth < MAX_DEPTH; ++depth) {
    if (!intersectsScene(ray, isect)) return vec3(0.0f);

    // Direct light sampling in the first iteration.
    // Surfaces only emit light if they're being looked at from the front.
    // TODO(explicit light sampling)
    if (/*depth == 0 &&*/ !isect.backface) {
      color += beta * AreaLights[Meshes[isect.meshID].areaLightID].emission;
    }
    beta *= Materials[Meshes[isect.meshID].materialID].kd;

    // Explicit light source sampling.
    // Don't do this for perfectly specular BSDFs.
    //color += beta * sampleLight(isect, );

    // Sample BSDF to get a new path direction.
    const float u1 = rand();
    const float u2 = rand();
    const float theta = 2.0f * M_PI * u1;
    const float phi = sqrt(u2);

    const vec3 w = isect.normal;
    const vec3 u = normalize(
        cross(abs(w.x) >= 0.1f + EPSILON ? vec3(0, 1, 0) : vec3(1, 0, 0), w));
    const vec3 v = cross(w, u);

    const vec3 direction = normalize(
        u * cos(theta) * phi +
        v * sin(theta) * phi +
        w * sqrt(1.0f - u2));

    ray = Ray(isect.point + isect.normal * 0.03, direction);
  }

  return color;
}


#endif // !HERAKLES_SHADERS_PATH_TRACER_GLSL
