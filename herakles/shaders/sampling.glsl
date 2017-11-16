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
#include "utils.glsl"

/// Returns the (s, t) barycentric coordinates of an uniform triangle sample.
/// PBRTv3 page 781.
vec2 uniformTriangleST() {
  const float u0 = rand();
  const float u1 = rand();
  const float su0 = sqrt(u0);
  return vec2(1 - su0, u1 * su0);
}

/// PDF of an uniformly sampled cone.
float uniformConePdf(float cosThetaMax) {
  return 1.0f / (2.0f * M_PI * (1.0f - cosThetaMax));
}

/// Samples a cone going in the z direction uniformly.
vec3 uniformSampleCone(float cosThetaMax, const vec3 x, const vec3 y,
                       const vec3 z) {
  const float cosTheta = lerp(rand(), cosThetaMax, 1.0f);
  const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
  const float phi = rand() * 2 * M_PI;
  return sphericalDirection(sinTheta, cosTheta, phi, x, y, z);
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
/// uniformly.
/// Returns if there is any light contribution to isect or not. If there is, the
/// contribution output is set to the light contribution to the intersection.
/// Be sure the number of area lights is > 1 when calling this.
bool sampleOneAreaLight(const uint areaLightIndex, const Interaction isect,
                        const float pdf, out vec3 contribution) {
  const AreaLight light = AreaLights[areaLightIndex];
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

  const float dist2 = dot(unormDir, unormDir);
  const float lightPdf = triangleArea(begin) * absDot(isect.normal, dir)
                       * absDot(triangleIt.normal, -1.0f * dir) / (pdf * dist2);

  if (unoccluded(Ray(isect.point, dir), sqrt(dist2),
                 SkipTriangle(true, isect.meshID, isect.begin))) {
    contribution = light.emission * lightPdf;
    return true;
  }

  return false;
}

float spotLightFalloff(const SpotLight light, const vec3 invDir) {
  const vec3 axis = normalize(light.to - light.from);
  const float cosTheta = dot(invDir, axis);

  if (cosTheta < light.cosTotalWidth) return 0.0f;
  if (cosTheta >= light.cosFalloffStart) return 1.0f;

  const float delta = (cosTheta - light.cosTotalWidth) /
                      (light.cosFalloffStart - light.cosTotalWidth);

  return (delta * delta) * (delta * delta);
} 

/// Uniformly samples one spot light. Returns if there is any contribution to
/// isect or not, with the potential contribution set in the contribution
/// variable.
/// Be sure the number of spot lights is > 1 when calling this.
bool sampleOneSpotLight(const uint spotLightIndex, const Interaction isect,
                        const float pdf, out vec3 contribution) {
  const SpotLight light = SpotLights[spotLightIndex];
  const vec3 unormDir = light.from - isect.point;
  const vec3 dir = normalize(unormDir);
  const float dist2 = dot(unormDir, unormDir);

  if (unoccluded(Ray(isect.point, dir), sqrt(dist2),
                 SkipTriangle(true, isect.meshID, isect.begin))) {
    const float falloff = spotLightFalloff(light, -1.0f * dir);
    contribution = light.emission * falloff * absDot(isect.normal, dir)
                 / (pdf * dist2);
    return true;
  }

  return false;
}

bool sampleOneLight(const Interaction isect, out vec3 contribution) {
  const uint numAreaLights = AreaLights.length();
  const uint numSpotLights = SpotLights.length();
  const uint numLights = numAreaLights + numSpotLights;
  if (numLights == 0) return false;

  const uint lightIndex = urand(numLights);
  const float pdf = float(numLights) + (HasAmbientLight ? 1.0f : 0.0f);
  if (lightIndex < numAreaLights) {
    return sampleOneAreaLight(lightIndex, isect, pdf, contribution);
  } else {
    return sampleOneSpotLight(lightIndex - numAreaLights, isect, pdf,
                              contribution);
  }
}

/// Samples an area light source for emitted light.
vec3 sampleAreaLightEmission(
    const uint areaLightIndex, out Ray ray, out vec3 normal, out float pdfPos,
    out float pdfDir) {
  // TODO(renatoutsch): implement this.
  return vec3(0.0f);
}

/// Samples an area light source's PDF. Returns if there's any contribution.
bool sampleAreaLightPdf(
      const uint areaLightIndex, const Ray ray, const vec3 normal,
      out float pdfPos, out float pdfDir) {
  // TODO(renatoutsch): implement this.
  return false;
}

/// Samples a spot light source for emitted light.
vec3 sampleSpotLightEmission(
      const uint spotLightIndex, out Ray ray, out vec3 normal, out float pdfPos,
      out float pdfDir) {
  const SpotLight light = SpotLights[spotLightIndex];

  const vec3 axis = normalize(light.to - light.from);
  vec3 dx, dy;
  coordinateSystem(axis, dx, dy);

  const vec3 w = uniformSampleCone(light.cosTotalWidth, dx, dy, axis);
  ray = Ray(light.from, w);
  normal = ray.direction;
  pdfPos = 1.0f;
  pdfDir = uniformConePdf(light.cosTotalWidth);
  return light.emission * spotLightFalloff(light, ray.direction);
}

/// Samples a spot light source's PDF. Returns if there is any contribution.
bool sampleSpotLightPdf(
      const uint spotLightIndex, const Ray ray, const vec3 normal,
      out float pdfPos, out float pdfDir) {
  const SpotLight light = SpotLights[spotLightIndex];
  const vec3 axis = normalize(light.to - light.from);
  const float cosTheta = dot(-1.0f * ray.direction, axis);

  if (cosTheta >= light.cosTotalWidth) {
    pdfPos = 1.0f;
    pdfDir = uniformConePdf(light.cosTotalWidth)
           / spotLightFalloff(light, -1.0f * ray.direction);
    return true;
  }

  return false;
}

/// Samples a random light source for emitted light.
vec3 sampleLightEmission(
      out uint lightIndex, out Ray ray, out vec3 normal, out float pdfLight,
      out float pdfPos, out float pdfDir) {
  const uint numAreaLights = AreaLights.length();
  const uint numSpotLights = SpotLights.length();
  const uint numLights = numAreaLights + numSpotLights;
  if (numLights == 0) return vec3(0.0f);

  lightIndex = urand(numLights);
  pdfLight = float(numLights) + (HasAmbientLight ? 1.0f : 0.0f);
  if (lightIndex < numAreaLights) {
    return sampleAreaLightEmission(lightIndex, ray, normal, pdfPos, pdfDir);
  } else {
    return sampleSpotLightEmission(lightIndex - numAreaLights, ray, normal,
                                   pdfPos, pdfDir);
  }
}

/// Samples a random light Pdf. Assumes there are lights to sample, except
/// ambient light. Returns if there is any contribution.
bool sampleLightPdf(const Ray ray, const vec3 normal, const uint lightIndex,
                    out float pdfLight, out float pdfPos, out float pdfDir) {
  const uint numAreaLights = AreaLights.length();
  const uint numSpotLights = SpotLights.length();
  const uint numLights = numAreaLights + numSpotLights;
  pdfLight = float(numLights) + (HasAmbientLight ? 1.0f : 0.0f);

  if (lightIndex < numAreaLights) {
    return sampleAreaLightPdf(lightIndex, ray, normal, pdfPos, pdfDir);
  } else {
    return sampleSpotLightPdf(lightIndex - numAreaLights, ray, normal,
                              pdfPos, pdfDir);
  }
}

#endif // !HERAKLES_SHADERS_SAMPLING_GLSL
