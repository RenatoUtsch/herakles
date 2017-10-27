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

#ifndef HERAKLES_HERAKLES_SCENE_BVH_HPP
#define HERAKLES_HERAKLES_SCENE_BVH_HPP

#include <cstdint>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

#include "herakles/scene/bounds.hpp"
#include "herakles/scene/scene_generated.h"

namespace hk {

/**
 * Representation of a triangle used in the BVH.
 */
struct BVHTriangle {
  /// ID of this triangle's mesh.
  uint32_t meshID;

  /// Beginning of this triangle in the indices array.
  uint32_t begin;

  BVHTriangle(uint32_t meshID, uint32_t begin) : meshID(meshID), begin(begin) {}
};

/**
 * A node of the BVH represented as an element in an array.
 * This struct has exactly 256bits, and is packed so that every component is
 * cache-aligned in the GPU for maximum performance.
 *
 * For an interior node, it's first child is always the next element in the
 * array, so the first child's index doesn't need to be stored, only the
 * second child's.
 */
struct BVHNode {
  /// First point that represents the minimum of the bounding box.
  glm::vec3 minPoint;

  /// Number of triangles in the node. If 0, the node is an interior node, and
  /// if > 0, the node is a leaf node.
  uint32_t numTriangles;

  /// Second point that represents the maximum of the bounding box.
  glm::vec3 maxPoint;

  union {
    /// If it's a leaf node, the offset into the triangles array.
    uint32_t trianglesOffset;

    /// If it's an internal node, the offset to the second child.
    uint32_t secondChildOffset;
  };
};

/// Struct that stores the BVH data.
struct BVHData {
  std::vector<BVHNode> nodes;

  std::vector<BVHTriangle> triangles;

  BVHData(std::vector<BVHNode> &&nodes, std::vector<BVHTriangle> &&triangles)
      : nodes(nodes), triangles(triangles) {}
};

/**
 * Builds a Bounding Volume Hierarchy from the given scene.
 * @return a pair containing the BVH tree vector and the BVH triangle vector.
 */
BVHData buildBVH(const hk::scene::Scene *scene);

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_SCENE_BVH_HPP
