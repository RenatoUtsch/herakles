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
#include "sampling.glsl"
#include "intersection.glsl"

/// Returns estimated radiance along ray.
vec3 radiance(Ray ray) {
  vec3 color = vec3(0.0f);
  vec3 beta = vec3(1.0f);
  Interaction isect;
  SkipTriangle skip = SkipTriangle(false, 0, 0);
  for (int depth = 0; depth < MAX_DEPTH; ++depth) {
    if (!intersectsScene(ray, skip, isect)) {
      // Poor man's excuse of an infinite area light.
      color += beta * ambientLight;
      break;
    }

    // Direct light sampling in the first iteration.
    // Surfaces only emit light if they're being looked at from the front.
    if (depth == 0 && !isect.backface) {
      const int areaLightID = Meshes[isect.meshID].areaLightID;
      if (areaLightID >= 0) { // Otherwise it doesn't emit.
        color += beta * AreaLights[areaLightID].emission;
        break;
      }
    }

    // Update the reflectance.
    beta *= Materials[Meshes[isect.meshID].materialID].kd;

    // Explicit light source sampling.
    // Don't do this for perfectly specular BSDFs.
    vec3 lightContribution;
    if (sampleOneLight(isect, lightContribution)) {
      color += beta * lightContribution;
    }

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

    ray.origin = isect.point;
    ray.direction = direction;
    skip = SkipTriangle(true, isect.meshID, isect.begin);
  }

  return color;
}


#endif // !HERAKLES_SHADERS_PATH_TRACER_GLSL
