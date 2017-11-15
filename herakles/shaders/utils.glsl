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

#endif // !HERAKLES_SHADERS_UTILS_GLSL
