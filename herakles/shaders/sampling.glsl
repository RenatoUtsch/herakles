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
 * Sampling related functions.
 */

#ifndef HERAKLES_SHADERS_SAMPLING_GLSL
#define HERAKLES_SHADERS_SAMPLING_GLSL

#include "random.glsl"
#include "scene.glsl"
#include "intersection.glsl"

/// Returns the (s, t) barycentric coordinates of an uniform triangle sample.
/// PBRTv3 page 781.
vec2 uniformTriangleST() {
  const float u0 = rand();
  const float u1 = rand();
  const float su0 = sqrt(u0);
  return vec2(1 - su0, u1 * su0);
}

/// Uniformly samples a point in a triangle.
/// Returns the interaction. This interaction has an unoriented normal and
/// 'backface' is always false. It's your responsibility to check for
/// orientation.
Interaction sampleTriangle(const uint meshID, const uint begin) {
  const vec2 b = uniformTriangleST();
  const float p = (1.0f - b.s - b.t);
  const vec3 point = b.s * Vertices[Indices[begin]]
                   + b.t * Vertices[Indices[begin + 1]]
                   + p   * Vertices[Indices[begin + 2]];
  const vec3 normal = b.s * Normals[Indices[begin]]
                    + b.t * Normals[Indices[begin + 1]]
                    + p   * Normals[Indices[begin + 2]];

  return Interaction(point, meshID, normal, false, begin);
}

/// Uniformly samples one area light source. The area light source is chosen
/// uniformly, excluding the first, unused area light.
/// Returns if there is any light contribution to isect or not. If there is, the
/// contribution output is set to the light contribution to the intersection.
/// Be sure the number of area lights is > 1 when calling this.
bool sampleOneLight(const Interaction isect, out vec3 contribution) {
  const AreaLight light = AreaLights[urand(AreaLights.length())];
  const Mesh mesh = Meshes[light.meshID];

  // Chooses a triangle from the mesh at random.
  // TODO(renatoutsch): maybe take the area of the triangles into account.
  const uint numTriangles = (mesh.end - mesh.begin) / 3;
  const uint begin = mesh.begin + 3 * urand(numTriangles);

  // Chooses a point in the triangle at random.
  Interaction triangleIt = sampleTriangle(light.meshID, begin);
  const vec3 unormDir = triangleIt.point - isect.point;
  const vec3 dir = normalize(unormDir);

  // If backface, the light won't contribute anything, we're looking the wrong
  // way.
  if (dot(triangleIt.normal, dir) > -EPSILON) {
    return false;
  }

  const float dist2 = unormDir.x * unormDir.x
                    + unormDir.y * unormDir.y
                    + unormDir.z * unormDir.z;

  const float lightPdf = triangleArea(begin)
                       * dot(triangleIt.normal, -1.0f * dir) / dist2;

  if (unoccluded(Ray(isect.point, dir), sqrt(dist2),
                 SkipTriangle(true, isect.meshID, isect.begin))) {
    contribution = light.emission * lightPdf;
    return true;
  }

  return false;
}

#endif // !HERAKLES_SHADERS_SAMPLING_GLSL
