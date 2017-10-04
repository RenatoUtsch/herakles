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

  //uint transformID;
};

/**
 * Represents a single mesh's material. 
 */
struct Material {
  /// Reflectivity of the diffuse surface.
  vec3 kd;
};

layout(binding = 0, rgba32f) uniform restrict image2D Image;
layout(binding = 1, r32ui) uniform restrict uimage2D Seeds;
layout(binding = 2, std140) uniform restrict readonly UBO {
  PinholeCamera Camera;
  uint FrameCount;
};

/* layout(std430, binding = 3) buffer MeshesBuffer { */
/*   Mesh Meshes[]; */
/* }; */

/* layout(std430, binding = 4) buffer IndicesBuffer { */
/*   uint Indices[]; */
/* }; */

/* layout(std430, binding = 5) buffer VerticesBuffer { */
/*   vec3 Vertices[]; */
/* }; */

/* layout(std430, binding = 6) buffer NormalsBuffer { */
/*   vec3 Normals[]; */
/* }; */

/* layout(std430, binding = 7) buffer UVBuffer { */
/*   vec2 UV[]; */
/* }; */

/* layout(std430, binding = 8) buffer MaterialsBuffer { */
/*   Material Materials[]; */
/* }; */

/* layout(std430, binding = 9) buffer TransformsBuffer { */
/*   mat4 Transforms[]; */
/* }; */

/* Integrator "path" "integer maxdepth" [ 65 ] */
/* Transform [ 1 -0 -0 -0 -0 1 -0 -0 -0 -0 -1 -0 -0 -1 6.8 1] */
/* Sampler "sobol" "integer pixelsamples" [ 64 ] */
/* PixelFilter "triangle" "float xwidth" [ 1.000000 ] "float ywidth" [ 1.000000 ] */
/* Film "image" "integer xresolution" [ 1024 ] "integer yresolution" [ 1024 ] "string filename" [ "cornell-box.png" ] */
/* Camera "perspective" "float fov" [ 19.500000 ] */

const uint NUM_AREA_LIGHTS = 1;
const uint NUM_MATERIALS = 8;
const uint NUM_MESHES = 8;
const uint NUM_INDICES = 108;
const uint NUM_POINTS = 72;

const AreaLight AreaLights[NUM_AREA_LIGHTS] = AreaLight[](
  AreaLight(vec3(17, 12, 4), 7)
);

const Mesh Meshes[NUM_MESHES] = Mesh[](
  Mesh(0, 6, 0),
  Mesh(6, 12, 1),
  Mesh(12, 18, 2),
  Mesh(18, 24, 3),
  Mesh(24, 30, 4),
  Mesh(30, 36, 5),
  Mesh(66, 72, 6),
  Mesh(102, 108, 7)
);

const Material Materials[NUM_MATERIALS] = Material[](
  Material(vec3(0.630000, 0.065000, 0.050000)),
  Material(vec3(0.140000, 0.450000, 0.091000)),
  Material(vec3(0.725000, 0.710000, 0.680000)),
  Material(vec3(0.725000, 0.710000, 0.680000)),
  Material(vec3(0.725000, 0.710000, 0.680000)),
  Material(vec3(0.725000, 0.710000, 0.680000)),
  Material(vec3(0.725000, 0.710000, 0.680000)),
  Material(vec3(0.000000, 0.000000, 0.000000))
);

const uint Indices[NUM_INDICES] = uint[](
  // Floor
  0, 1, 2,
  0, 2, 3,
  // Ceiling
  4, 5, 6,
  4, 6, 7,
  // BackWall
  8, 9, 10,
  8, 10, 11,
  // RightWall
  12, 13, 14,
  12, 14, 15,
  // LeftWall
  16, 17, 18,
  16, 18, 19,
  // ShortBox
  20, 22, 21,
  20, 23, 22,
  24, 26, 25,
  24, 27, 26,
  28, 30, 29,
  28, 31, 30,
  32, 34, 33,
  32, 35, 34,
  36, 38, 37,
  36, 39, 38,
  40, 42, 41,
  40, 43, 42,
  // TallBox
  44, 46, 45,
  44, 47, 46,
  48, 50, 49,
  48, 51, 50,
  52, 54, 53,
  52, 55, 54,
  56, 58, 57,
  56, 59, 58,
  60, 62, 61,
  60, 63, 62,
  64, 66, 65,
  64, 67, 66,
  // Light
  68, 69, 70,
  68, 70, 71
);

const vec3 Points[NUM_POINTS] = vec3[](
  // Floor
  vec3(-1, 1.74846e-007, -1),
  vec3(-1, 1.74846e-007, 1),
  vec3(1, -1.74846e-007, 1),
  vec3(1, -1.74846e-007, -1),
  // Ceiling
  vec3(1, 2, 1),
  vec3(-1, 2, 1),
  vec3(-1, 2, -1),
  vec3(1, 2, -1),
  // BackWall
  vec3(-1, 0, -1),
  vec3(-1, 2, -1),
  vec3(1, 2, -1),
  vec3(1, 0, -1),
  // RightWall
  vec3(1, 0, -1),
  vec3(1, 2, -1),
  vec3(1, 2, 1),
  vec3(1, 0, 1),
  // LeftWall
  vec3(-1, 0, 1),
  vec3(-1, 2, 1),
  vec3(-1, 2, -1),
  vec3(-1, 0, -1),
  // ShortBox
  vec3(-0.0460751, 0.6, 0.573007),
  vec3(-0.0460751, -2.98023e-008, 0.573007),
  vec3(0.124253, 0, 0.00310463),
  vec3(0.124253, 0.6, 0.00310463),
  vec3(0.533009, 0, 0.746079),
  vec3(0.533009, 0.6, 0.746079),
  vec3(0.703337, 0.6, 0.176177),
  vec3(0.703337, 2.98023e-008, 0.176177),
  vec3(0.533009, 0.6, 0.746079),
  vec3(-0.0460751, 0.6, 0.573007),
  vec3(0.124253, 0.6, 0.00310463),
  vec3(0.703337, 0.6, 0.176177),
  vec3(0.703337, 2.98023e-008, 0.176177),
  vec3(0.124253, 0, 0.00310463),
  vec3(-0.0460751, -2.98023e-008, 0.573007),
  vec3(0.533009, 0, 0.746079),
  vec3(0.533009, 0, 0.746079),
  vec3(-0.0460751, -2.98023e-008, 0.573007),
  vec3(-0.0460751, 0.6, 0.573007),
  vec3(0.533009, 0.6, 0.746079),
  vec3(0.703337, 0.6, 0.176177),
  vec3(0.124253, 0.6, 0.00310463),
  vec3(0.124253, 0, 0.00310463),
  vec3(0.703337, 2.98023e-008, 0.176177),
  // TallBox
  vec3(-0.720444, 1.2, -0.473882),
  vec3(-0.720444, 0, -0.473882),
  vec3(-0.146892, 0, -0.673479),
  vec3(-0.146892, 1.2, -0.673479),
  vec3(-0.523986, 0, 0.0906493),
  vec3(-0.523986, 1.2, 0.0906492),
  vec3(0.0495656, 1.2, -0.108948),
  vec3(0.0495656, 0, -0.108948),
  vec3(-0.523986, 1.2, 0.0906492),
  vec3(-0.720444, 1.2, -0.473882),
  vec3(-0.146892, 1.2, -0.673479),
  vec3(0.0495656, 1.2, -0.108948),
  vec3(0.0495656, 0, -0.108948),
  vec3(-0.146892, 0, -0.673479),
  vec3(-0.720444, 0, -0.473882),
  vec3(-0.523986, 0, 0.0906493),
  vec3(-0.523986, 0, 0.0906493),
  vec3(-0.720444, 0, -0.473882),
  vec3(-0.720444, 1.2, -0.473882),
  vec3(-0.523986, 1.2, 0.0906492),
  vec3(0.0495656, 1.2, -0.108948),
  vec3(-0.146892, 1.2, -0.673479),
  vec3(-0.146892, 0, -0.673479),
  vec3(0.0495656, 0, -0.108948),
  // Light
  vec3(-0.24, 1.98, -0.22),
  vec3(0.23, 1.98, -0.22),
  vec3(0.23, 1.98, 0.16),
  vec3(-0.24, 1.98, 0.16)
);

const vec3 Normals[NUM_POINTS] = vec3[](
  // Floor
  vec3(4.37114e-008, 1, 1.91069e-015),
  vec3(4.37114e-008, 1, 1.91069e-015),
  vec3(4.37114e-008, 1, 1.91069e-015),
  vec3(4.37114e-008, 1, 1.91069e-015),
  // Ceiling
  vec3(-8.74228e-008, -1, -4.37114e-008),
  vec3(-8.74228e-008, -1, -4.37114e-008),
  vec3(-8.74228e-008, -1, -4.37114e-008),
  vec3(-8.74228e-008, -1, -4.37114e-008),
  // BackWall
  vec3(8.74228e-008, -4.37114e-008, -1),
  vec3(8.74228e-008, -4.37114e-008, -1),
  vec3(8.74228e-008, -4.37114e-008, -1),
  vec3(8.74228e-008, -4.37114e-008, -1),
  // RightWall
  vec3(1, -4.37114e-008, 1.31134e-007),
  vec3(1, -4.37114e-008, 1.31134e-007),
  vec3(1, -4.37114e-008, 1.31134e-007),
  vec3(1, -4.37114e-008, 1.31134e-007),
  // LeftWall
  vec3(-1, -4.37114e-008, -4.37114e-008),
  vec3(-1, -4.37114e-008, -4.37114e-008),
  vec3(-1, -4.37114e-008, -4.37114e-008),
  vec3(-1, -4.37114e-008, -4.37114e-008),
  // ShortBox
  vec3(-0.958123, -4.18809e-008, -0.286357),
  vec3(-0.958123, -4.18809e-008, -0.286357),
  vec3(-0.958123, -4.18809e-008, -0.286357),
  vec3(-0.958123, -4.18809e-008, -0.286357),
  vec3(0.958123, 4.18809e-008, 0.286357),
  vec3(0.958123, 4.18809e-008, 0.286357),
  vec3(0.958123, 4.18809e-008, 0.286357),
  vec3(0.958123, 4.18809e-008, 0.286357),
  vec3(-4.37114e-008, 1, -1.91069e-015),
  vec3(-4.37114e-008, 1, -1.91069e-015),
  vec3(-4.37114e-008, 1, -1.91069e-015),
  vec3(-4.37114e-008, 1, -1.91069e-015),
  vec3(4.37114e-008, -1, 1.91069e-015),
  vec3(4.37114e-008, -1, 1.91069e-015),
  vec3(4.37114e-008, -1, 1.91069e-015),
  vec3(4.37114e-008, -1, 1.91069e-015),
  vec3(-0.286357, -1.25171e-008, 0.958123),
  vec3(-0.286357, -1.25171e-008, 0.958123),
  vec3(-0.286357, -1.25171e-008, 0.958123),
  vec3(-0.286357, -1.25171e-008, 0.958123),
  vec3(0.286357, 1.25171e-008, -0.958123),
  vec3(0.286357, 1.25171e-008, -0.958123),
  vec3(0.286357, 1.25171e-008, -0.958123),
  vec3(0.286357, 1.25171e-008, -0.958123),
  // TallBox
  vec3(-0.328669, -4.1283e-008, -0.944445),
  vec3(-0.328669, -4.1283e-008, -0.944445),
  vec3(-0.328669, -4.1283e-008, -0.944445),
  vec3(-0.328669, -4.1283e-008, -0.944445),
  vec3(0.328669, 4.1283e-008, 0.944445),
  vec3(0.328669, 4.1283e-008, 0.944445),
  vec3(0.328669, 4.1283e-008, 0.944445),
  vec3(0.328669, 4.1283e-008, 0.944445),
  vec3(3.82137e-015, 1, -4.37114e-008),
  vec3(3.82137e-015, 1, -4.37114e-008),
  vec3(3.82137e-015, 1, -4.37114e-008),
  vec3(3.82137e-015, 1, -4.37114e-008),
  vec3(-3.82137e-015, -1, 4.37114e-008),
  vec3(-3.82137e-015, -1, 4.37114e-008),
  vec3(-3.82137e-015, -1, 4.37114e-008),
  vec3(-3.82137e-015, -1, 4.37114e-008),
  vec3(-0.944445, 1.43666e-008, 0.328669),
  vec3(-0.944445, 1.43666e-008, 0.328669),
  vec3(-0.944445, 1.43666e-008, 0.328669),
  vec3(-0.944445, 1.43666e-008, 0.328669),
  vec3(0.944445, -1.43666e-008, -0.328669),
  vec3(0.944445, -1.43666e-008, -0.328669),
  vec3(0.944445, -1.43666e-008, -0.328669),
  vec3(0.944445, -1.43666e-008, -0.328669),
  // Light
  vec3(-8.74228e-008, -1, 1.86006e-007),
  vec3(-8.74228e-008, -1, 1.86006e-007),
  vec3(-8.74228e-008, -1, 1.86006e-007),
  vec3(-8.74228e-008, -1, 1.86006e-007)
);

const vec2 UVs[NUM_POINTS] = vec2[](
  // Floor
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // Ceiling
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // BackWall
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // RightWall
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // LeftWall
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // ShortBox
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // TallBox
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1),
  // Light
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1)
);

#endif // !HERAKLES_SHADERS_SCENE_GLSL
