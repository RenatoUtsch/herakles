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

#include "bsdf.glsl"
#include "intersection.glsl"
#include "random.glsl"
#include "scene.glsl"
#include "sampling.glsl"
#include "utils.glsl"

/// Returns estimated radiance along ray.
vec3 pathTracingRadiance(Ray ray) {
  vec3 color = vec3(0.0f);
  vec3 beta = vec3(1.0f);
  bool perfectlySpecularBounce = false;
  Interaction isect;
  SkipTriangle skip = SkipTriangle(false, 0, 0);
  for (uint depth = 0; depth < CameraPathLength; ++depth) {
    if (!intersectsScene(ray, skip, isect)) {
      // Poor man's excuse of an infinite area light.
      if (HasAmbientLight) {
        color += beta * AmbientLight;
      } else {
        color = vec3(0.0f);
      }
      return color;
    }

    // Direct light sampling in the first iteration.
    // Surfaces only emit light if they're being looked at from the front.
    if ((depth == 0 || perfectlySpecularBounce) && !isect.backface) {
      const int areaLightID = Meshes[isect.meshID].areaLightID;
      if (areaLightID >= 0) { // Otherwise it doesn't emit.
        color += beta * AreaLights[areaLightID].emission;
        break;
      }
    }

    // Sample BSDF to get a new path direction.
    vec3 wi;
    float pdf;
    const vec3 f = sampleBSDF(isect, ray.direction, wi, pdf,
                              perfectlySpecularBounce);

    // Update the reflectance.
    beta *= f * absDot(wi, isect.normal) / pdf;

    // Explicit light source sampling.
    // Don't do this for perfectly specular BSDFs.
    vec3 lightContribution;
    if (!perfectlySpecularBounce && sampleOneLight(isect, lightContribution)) {
      color += beta * lightContribution;
    }

    ray = Ray(isect.point, wi);
    skip = SkipTriangle(true, isect.meshID, isect.begin);
  }

  return color;
}


#endif // !HERAKLES_SHADERS_PATH_TRACER_GLSL
