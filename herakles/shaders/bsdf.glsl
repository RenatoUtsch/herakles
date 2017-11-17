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

#include "extensions.glsl"
#include "random.glsl"
#include "scene.glsl"
#include "utils.glsl"

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
  pdf = absDot(wi, isect.normal) * M_1_PI;
  perfectlySpecular = false;
  return M_1_PI * material.kr;
}

float fresnelDielectric(const float cosThetaI, const float etaI,
                        const float etaT) {
  const float sinThetaI = sqrt(max(0.0f, 1.0f - cosThetaI * cosThetaI));
  const float sinThetaT = etaI / etaT * sinThetaI;

  if (sinThetaT >= 1.0f) {  // Total internal reflection.
    return 1.0f;
  }

  const float cosThetaT = sqrt(max(0.0f, 1.0f - sinThetaT * sinThetaT));

  const float etaTthetaI = etaT * cosThetaI;
  const float etaIthetaT = etaI * cosThetaT;
  const float etaIthetaI = etaI * cosThetaI;
  const float etaTthetaT = etaT * cosThetaT;
  const float Rparl = (etaTthetaI - etaIthetaT) / (etaTthetaI + etaIthetaT);
  const float Rperp = (etaIthetaI - etaTthetaT) / (etaIthetaI + etaTthetaT);

  return (Rparl * Rparl + Rperp * Rperp) / 2.0f;
}

vec3 sampleGlass(const Interaction isect, const Material material,
                 const vec3 invWo, out vec3 wi, out float pdf,
                 out bool perfectlySpecular) {
  const float cosDirNormal = clamp(dot(-1.0f * invWo, isect.normal), 0.0f, 1.0f);
  float etaI = 1.0f, etaT = material.eta;
  if (isect.backface) {  // Guarantee etaI has the IOR of the incident medium.
    swap(etaI, etaT);
  }

  perfectlySpecular = true;
  const float f = fresnelDielectric(cosDirNormal, etaI, etaT);
  if (rand() < f) {  // Specular reflection
    wi = reflect(invWo, isect.normal);
    pdf = f;
    return f * material.kr / absDot(wi, isect.normal);
  } else {  // Specular transmission
    wi = refract(invWo, isect.normal, etaI / etaT);
    pdf = 1.0f - f;
    return  (1.0f - f) * material.kt / absDot(wi, isect.normal);
  }
}

vec3 sampleMirror(const Interaction isect, const Material material,
                  const vec3 invWo, out vec3 wi, out float pdf,
                  out bool perfectlySpecular) {
  wi = reflect(invWo, isect.normal);
  pdf = 1.0f;
  perfectlySpecular = true;
  return material.kr / absDot(wi, isect.normal);
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
