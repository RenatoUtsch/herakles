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
 * Utility functions.
 */

#ifndef HERAKLES_SHADERS_UTILS_GLSL
#define HERAKLES_SHADERS_UTILS_GLSL

float absDot(const vec3 v1, const vec3 v2) {
  return abs(dot(v1, v2));
}

void swap(inout float a, inout float b) {
  const float c = a;
  a = b;
  b = c;
}

float lerp(float t, float p0, float p1) {
  return (1.0f - t) * p0 + t * p1;
}

// Creates a coordinate system for the given vector.
void coordinateSystem(const vec3 v1, out vec3 v2, out vec3 v3) {
  if (abs(v1.x) > abs(v1.y)) {
    v2 = vec3(-1.0f * v1.z, 0.0f, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
  } else {
    v2 = vec3(0.0f, v1.z, -1.0f * v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
  }
  v3 = cross(v1, v2);
}

// Converts from spherical coordinates to the given axis.
vec3 sphericalDirection(float sinTheta, float cosTheta, float phi,
                        const vec3 x, const vec3 y, const vec3 z) {
 return cos(phi) * sinTheta * x + sin(phi) * sinTheta * y + cosTheta * z;
}

#endif // !HERAKLES_SHADERS_UTILS_GLSL
