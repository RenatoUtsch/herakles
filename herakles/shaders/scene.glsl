/**
 * Scene structures and functions to simplify using Herakles scenes in GLSL.
 */

struct Mesh {
  uint beginIndex;
  uint endIndex;
  uint materialID;
  uint transformID;
};

struct Material {
  /// Reflectivity of the diffuse surface.
  vec3 kd;
};

layout(std430, binding = 0) buffer MeshesBuffer {
  Mesh Meshes[];
};

layout(std430, binding = 1) buffer IndicesBuffer {
  uint Indices[];
};

layout(std430, binding = 2) buffer VerticesBuffer {
  vec3 Vertices[];
};

layout(std430, binding = 3) buffer NormalsBuffer {
  vec3 Normals[];
};

layout(std430, binding = 4) buffer UVBuffer {
  vec2 UV[];
};

layout(std430, binding = 5) buffer MaterialsBuffer {
  Material Materials[];
};

layout(std430, binding = 6) buffer TransformsBuffer {
  mat4 Transforms[];
};
