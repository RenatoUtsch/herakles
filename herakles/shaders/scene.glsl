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

#ifndef HERAKLES_SHADERS_SCENE_GLSL
#define HERAKLES_SHADERS_SCENE_GLSL

// (u, v) = (s, t) for coordinates in Herakles.

const uint NUM_SAMPLES = 5;
const uint MAX_DEPTH = 4;

const float EPSILON = 1e-7;
const float INF = 1e20;
const float M_PI = 3.14159265359f;

/**
 * Represents a ray travelling through the scene.
 */
struct Ray {
  vec3 origin;
  vec3 direction;
};

/**
 * Pinhole camera used in the scene.
 */
struct PinholeCamera {
  vec3 position;
  float fov;
  vec3 direction;
  vec3 up;
  vec3 right;
};

/**
 * Represents a single area light.
 * The first area light represents no light, and is always skipped. It must
 * always have emission of vec3(0, 0, 0).
 */
struct AreaLight {
  vec3 emission;
  uint meshID;
};

/**
 * Represents a single mesh instance.
 */
struct Mesh {
  /// Starting index in the indices array.
  uint begin;

  /// One past last index in the indices array.
  uint end;

  /// Index of the material of this mesh.
  uint materialID;

  /// Index of the area light of this mesh. If 0, the mesh doesn't emit light,
  /// but the area light 0 emission is always 0, so you don't have to make a
  /// special case.
  int areaLightID;

  //uint transformID;
};

/**
 * Represents a single mesh's material. 
 */
struct Material {
  /// Reflectivity of the diffuse surface.
  vec3 kd;
};

/**
 * Represents an interaction with a triangle point.
 */
struct Interaction {
  /// Interaction point.
  vec3 point;

  /// Mesh ID.
  uint meshID;

  /// Oriented intersection normal.
  vec3 normal;

  /// If the intersection came from the backface. If this is true, the normal
  /// has already been inversed automatically.
  bool backface;
};

layout(binding = 0, rgba32f) uniform restrict image2D Image;
layout(binding = 1, rg32ui) uniform restrict uimage2D Seeds;
layout(binding = 2, std140) uniform restrict readonly UBO {
  PinholeCamera Camera;
  uint FrameCount;
};

layout(std430, binding = 3) buffer AreaLightBuffer {
  AreaLight AreaLights[];
};

layout(std430, binding = 4) buffer MeshesBuffer {
  Mesh Meshes[];
};

layout(std430, binding = 5) buffer MaterialsBuffer {
  Material Materials[];
};

layout(std430, binding = 6) buffer IndicesBuffer {
  uint Indices[];
};

layout(std430, binding = 7) buffer VerticesBuffer {
  vec3 Vertices[];
};

layout(std430, binding = 8) buffer NormalsBuffer {
  vec3 Normals[];
};

layout(std430, binding = 9) buffer UVBuffer {
  vec2 UVs[];
};

/* layout(std430, binding = 10) buffer TransformsBuffer { */
/*   mat4 Transforms[]; */
/* }; */

#endif // !HERAKLES_SHADERS_SCENE_GLSL
