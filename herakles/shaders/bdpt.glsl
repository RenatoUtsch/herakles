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
 * Scene structures and functions to simplify using Herakles scenes in GLSL.
 *
 * Do NOT change the order of the elements in these structs, unless you know
 * what you're doing. Changing this without changing the C++ code and scene file
 * format will break the code because you're changing the binary format.
 */

#ifndef HERAKLES_SHADERS_BDPT_GLSL
#define HERAKLES_SHADERS_BDPT_GLSL

#include "extensions.glsl"
#include "bsdf.glsl"
#include "intersection.glsl"
#include "random.glsl"
#include "scene.glsl"
#include "sampling.glsl"
#include "utils.glsl"

struct LightInteraction {
  Interaction isect;

  // Light index.
  uint lightIndex;

  // Path size.
  uint s;

  // If is connectible.
  bool connectible;

  // Light contribution.
  vec3 color;
};

bool connectSubpaths(const vec3 origin, const vec3 beta,
                     const SkipTriangle skip, const LightInteraction lightIsect,
                     out vec3 lightColor) {
  const vec3 unormDir = lightIsect.isect.point - origin;
  const vec3 dir = normalize(unormDir);
  const float dist2 = dot(unormDir, unormDir);
  const Ray ray = Ray(origin, dir);

  float pdf = 1.0f;
  if (lightIsect.s == 1) {
    // Sampling light directly. Need to get the light's pdf.
    float pdfLight, pdfPos, pdfDir;
    if (!sampleLightPdf(ray, lightIsect.isect.normal, lightIsect.lightIndex,
                        pdfLight, pdfPos, pdfDir)) {
      return false;
    }

    pdf = pdfLight * pdfPos * pdfDir * dist2;
  }

  lightColor = lightIsect.color * beta * absDot(dir, lightIsect.isect.normal)
             / pdf;
  if (isBlack(lightColor)) {
    return false;
  }

  // Connecting two paths, just need to check whether they are occluded.
  return unoccluded(ray, sqrt(dist2), skip);
}

/// Generates the light subpath.
LightInteraction generateLightSubpath() {
  uint lightIndex;
  Ray ray;
  vec3 normal;
  float pdfLight, pdfPos, pdfDir;
  const vec3 le = sampleLightEmission(lightIndex, ray, normal, pdfLight, pdfPos,
                                      pdfDir);
  if (le == vec3(0.0f)) {  // No lights to sample, return no contribution.
    return LightInteraction(Interaction(vec3(0.0f), 0, vec3(0.0f), false, 0),
                            lightIndex, 0, false, vec3(0.0f));
  }

  vec3 color = le * absDot(normal, ray.direction) / (pdfLight * pdfPos * pdfDir);

  uint s;
  Interaction oldIsect = Interaction(ray.origin, 0, normal, false, 0);
  bool perfectlySpecularBounce;
  SkipTriangle skip = SkipTriangle(false, 0, 0);
  for (s = 1; s < LightPathLength; ++s) {
    Interaction isect;
    if (!intersectsScene(ray, skip, isect)) {
      break;
    }

    // Sample BSDF to get a new path direction.
    vec3 wi;
    float pdf;
    const vec3 f = sampleBSDF(isect, ray.direction, wi, pdf,
                              perfectlySpecularBounce);

    // Compute distance to intersection if is the first ray.
    float dist2 = 1.0f;
    if (s == 1) {
      const vec3 unormDir = isect.point - ray.origin;
      dist2 = dot(unormDir, unormDir);
    }

    // TODO(renatoutsch): what if this is an area light??

    // Update the reflectance.
    color *= f * absDot(wi, isect.normal) / (pdf * dist2);

    ray = Ray(isect.point, wi);
    skip = SkipTriangle(true, isect.meshID, isect.begin);
    oldIsect = Interaction(isect.point, isect.meshID, isect.normal,
                           isect.backface, isect.begin);
  }

  return LightInteraction(oldIsect, lightIndex, s, !perfectlySpecularBounce,
                          color);
}

/// Generates and connects the camera subpath to the light subpath.
vec3 generateCameraSubpath(Ray ray, const LightInteraction lightIsect) {
  vec3 color = vec3(0.0f);
  vec3 beta = vec3(1.0f);
  bool perfectlySpecularBounce = false;
  vec3 lightColor;
  SkipTriangle skip = SkipTriangle(false, 0, 0);

  // TODO(renatoutsch): this doesn't work. The color needs to be splatted to the
  // correct pixel.
  if (lightIsect.connectible &&
      connectSubpaths(ray.origin, beta, skip, lightIsect, lightColor)) {
    color += lightColor;
  }

  uint t;
  Interaction isect;
  for(t = 1; t <= CameraPathLength; ++t) {
    if (!intersectsScene(ray, skip, isect)) {
      // Poor man's excuse of an infinite area light.
      if (HasAmbientLight) {
        color += beta * AmbientLight;
      }
      break;
    }
    skip = SkipTriangle(true, isect.meshID, isect.begin);

    const Mesh mesh = Meshes[isect.meshID];
    if (mesh.areaLightID >= 0) {
      // Area light. Assuming iteration doesn't continue after area lights.
      color += beta * AreaLights[mesh.areaLightID].emission;
      break;
    }

    // Sample BSDF to get a new path direction.
    vec3 wi;
    float pdf;
    const vec3 f = sampleBSDF(isect, ray.direction, wi, pdf,
                              perfectlySpecularBounce);

    // Update the reflectance.
    beta *= f / pdf;

    // Attempt to connect the current path to the light's subpath.
    if (!perfectlySpecularBounce && lightIsect.connectible &&
        connectSubpaths(isect.point, beta, skip, lightIsect, lightColor)) {
      color += lightColor;
    }

    beta *= absDot(wi, isect.normal);

    ray = Ray(isect.point, wi);
  }

  return color;
}

vec3 bdptRadiance(Ray ray) {
  const LightInteraction lightIsect = generateLightSubpath();
  return generateCameraSubpath(ray, lightIsect);
}

#endif // !HERAKLES_SHADERS_BDPT_GLSL
