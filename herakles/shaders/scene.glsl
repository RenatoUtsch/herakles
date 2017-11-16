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

// Rendering strategies.
const uint PathTracingStrategy = 0;
const uint BDPTStrategy = 1;

const uint RenderingStrategy = 1;
const uint NumSamples = 1;
const uint CameraPathLength = 8;
const uint LightPathLength = 1;  // Only useful for BDPT.

const float EPSILON = 1e-7;
const float INF = 1e20;
const float M_PI = 3.14159265358979323846;
const float M_1_PI = 0.31830988618379067154;

#define GAMMA(n) (((n) * EPSILON) / (1.0f - (n) * EPSILON))
const float GAMMA_3 = GAMMA(3.0f);

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
 * Represents a single spot light.
 */
struct SpotLight {
  vec3 emission;
  vec3 from;
  float cosTotalWidth;
  vec3 to;
  float cosFalloffStart;
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

/// Material types.
const uint MatteMaterial = 0;
const uint GlassMaterial = 1;
const uint MirrorMaterial = 2;

/**
 * Represents a single mesh's material. 
 */
struct Material {
  /// Reflectivity of the surface.
  vec3 kr;

  /// Material type.
  uint type;

  /// Transmissivity of the surface.
  vec3 kt;

  /// Index of refraction.
  float eta;
};

/**
 * Represents a triangle in the BVH.
 */
struct BVHTriangle {
  /// ID of the triangle's mesh.
  uint meshID;

  /// Beginning of the triangle's indices.
  uint begin;
};

/**
 * Represents a single BVH node in the GPU.
 */
struct BVHNode {
  /// First, minimum point in the BVH's bounding box.
  vec3 minPoint;

  /// 2 16bit integers backed into one variable. The number of triangles in the
  /// BVH node is packed into the first 16 bits and the split axis in the second
  /// 16 bits. Use unpackNumTrianglesAndAxis() to unpack this value.
  /// If numTriangles is 0, this is an internal node, otherwise it's a leaf
  /// node.
  uint packedNumTrianglesAndAxis;

  /// Second, maximum point in the BVH's bounding box.
  vec3 maxPoint;

  /// Union (at least in C++) of the trianglesOffset and secondChildOffset.
  /// If this is a leaf node, this is the index to the first triangle in the
  /// triangles array.
  /// If this is an internal node, this is the index to the second child of this
  /// node (the first child is the next element in the array).
  uint trianglesOrSecondChildOffset;
};

/// Unpacks the numTriangles and axis elements of a BVHNode.
/// This function assumes a Little Endian CPU.
void unpackNumTrianglesAndAxis(const BVHNode node, out uint numTriangles,
                                out uint axis) {
  // numTriangles is in the first 16 bits.
  numTriangles = node.packedNumTrianglesAndAxis & 0x0000FFFF;

  // axis is in the last 16 bits.
  axis = node.packedNumTrianglesAndAxis >> 16;
}

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
  /// has already been inverted automatically.
  bool backface;

  /// Triangle beginning.
  uint begin;
};

/**
 * Represents a triangle to be skipped.
 */
struct SkipTriangle {
  /// If should be skipped.
  bool skip;

  /// Index of the mesh.
  uint meshID;

  /// Beginning of the triangle.
  uint begin;
};

layout(binding = 0, rgba32f) uniform restrict image2D Image;
layout(binding = 1, rg32ui) uniform restrict uimage2D Seeds;
layout(binding = 2, std140) uniform restrict readonly UBO {
  PinholeCamera Camera;
  vec3 AmbientLight;
  bool HasAmbientLight;
  uint FrameCount;
};

layout(std430, binding = 3) buffer BVHNodeBuffer {
  BVHNode BVHNodes[];
};

layout(std430, binding = 4) buffer BVHTriangleBuffer {
  BVHTriangle BVHTriangles[];
};

layout(std430, binding = 5) buffer AreaLightBuffer {
  AreaLight AreaLights[];
};

layout(std430, binding = 6) buffer SpotLightBuffer {
  SpotLight SpotLights[];
};

layout(std430, binding = 7) buffer MeshesBuffer {
  Mesh Meshes[];
};

layout(std430, binding = 8) buffer MaterialsBuffer {
  Material Materials[];
};

layout(std430, binding = 9) buffer IndicesBuffer {
  uint Indices[];
};

layout(std430, binding = 10) buffer VerticesBuffer {
  vec3 Vertices[];
};

layout(std430, binding = 11) buffer NormalsBuffer {
  vec3 Normals[];
};

layout(std430, binding = 12) buffer UVBuffer {
  vec2 UVs[];
};

/* layout(std430, binding = 13) buffer TransformsBuffer { */
/*   mat4 Transforms[]; */
/* }; */

#endif // !HERAKLES_SHADERS_SCENE_GLSL
