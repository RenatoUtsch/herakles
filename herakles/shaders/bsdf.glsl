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
 * BSDF implementations.
 */

#ifndef HERAKLES_SHADERS_BSDF_GLSL
#define HERAKLES_SHADERS_BSDF_GLSL

#include "random.glsl"
#include "scene.glsl"

vec3 sampleMatte(const Interaction isect, const Material material,
                 const vec3 invWo, out vec3 wi, out float pdf,
                 out bool perfectlySpecular) {
  const float u1 = rand();
  const float u2 = rand();
  const float theta = 2.0f * M_PI * u1;
  const float phi = sqrt(u2);

  const vec3 w = isect.normal;
  const vec3 u = normalize(
      cross(abs(w.x) >= 0.1f + EPSILON ? vec3(0, 1, 0) : vec3(1, 0, 0), w));
  const vec3 v = cross(w, u);

  wi = normalize(u * cos(theta) * phi +
                 v * sin(theta) * phi +
                 w * sqrt(1.0f - u2));
  pdf = 1.0f;
  perfectlySpecular = false;
  return material.kr;
}

vec3 sampleGlass(const Interaction isect, const Material material,
                 const vec3 invWo, out vec3 wi, out float pdf,
                 out bool perfectlySpecular) {
  const vec3 unorientedNormal = isect.backface ? -1.0f * isect.normal
                                               : isect.normal;
  const vec3 reflected = reflect(invWo, unorientedNormal);
  const float nc = 1.0f, nt = material.eta;
  const float nnt = isect.backface ? nt / nc : nc / nt;
  const float ddn = dot(invWo, isect.normal);
  const float cos2t = 1.0f - nnt * nnt * (1.0f - ddn * ddn);

  // Total internal reflection
  perfectlySpecular = false;
  if (cos2t <= -EPSILON) {
    wi = reflected;
    pdf = 1.0f;
    return material.kr;
  }

  // Choose reflection or refraction
  const vec3 tdir = normalize(invWo * nnt - unorientedNormal * (
      ((isect.backface ? -1.0f : 1.0f) * (ddn * nnt + sqrt(cos2t)))));
  const float a = nt - nc;
  const float b = nt + nc;
  const float r0 = a * a / (b * b);
  const float c = 1.0f - (isect.backface ? dot(tdir, unorientedNormal) : -ddn);
  const float re = r0 + (1.0f - r0) * c * c * c * c * c;
  const float tr = 1.0f - re;
  const float p = 0.25f + 0.5f * re;

  if (rand() - p <= -EPSILON) { // Russian roulette
    wi = reflected;
    pdf = p;
    return material.kr * re;
  } else {
    wi = tdir;
    pdf = 1.0f - p;
    return material.kt * tr;
  }
}

vec3 sampleMirror(const Interaction isect, const Material material,
                  const vec3 invWo, out vec3 wi, out float pdf,
                  out bool perfectlySpecular) {
  wi = reflect(invWo, isect.normal);
  pdf = 1.0f;
  perfectlySpecular = true;
  return material.kr;
}

vec3 sampleBSDF(const Interaction isect, const vec3 invWo, out vec3 wi,
                out float pdf, out bool perfectlySpecular) {
  const Material material = Materials[Meshes[isect.meshID].materialID];
  if(material.type == MatteMaterial) {
    return sampleMatte(isect, material, invWo, wi, pdf, perfectlySpecular);
  } else if (material.type == GlassMaterial) {
    return sampleGlass(isect, material, invWo, wi, pdf, perfectlySpecular);
  } else if (material.type == MirrorMaterial) {
    return sampleMirror(isect, material, invWo, wi, pdf, perfectlySpecular);
  } else {
    return vec3(0.0f);  // Invalid material??
  }
}

#endif // !HERAKLES_SHADERS_BSDF_GLSL
