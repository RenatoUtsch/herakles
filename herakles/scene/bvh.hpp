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
#include <glm/glm.hpp>

namespace hk {

/**
 * Pointer-based representation of a node of the BVH.
 * Used to build the BVH, and later converted to the array representation.
 */
class BVHNode {
 public:
  /**
   * Builds an internal BVH node enclosing the two given leaf nodes.
   */
  BVHNode(BVHNode *leaf1, BVHNode *leaf2, uint16_t splitAxis)
      : bounds_(leaf1->bounds() + leaf2->bounds()),
        children_({leaf1, leaf2}),
        numPrimitives(0),
        splitAxis_(splitAxis),
        primitivesOffset_(0) {}

  /**
   * Builds a leaf BVH node by specifying the enclosed primitives.
   */
  BVHNode(const Bounds3f &bounds, uint16_t numPrimitives,
          uint32_t primitivesOffset)
      : bounds_(bounds),
        children_({nullptr, nullptr}),
        numPrimitives_(numPrimitives),
        splitAxis_(0),
        primitivesOffset_(primitiveOffset) {}

 private:
  /// Bounding box of the node.
  Bounds3f bounds_;

  /// Children of the node. Nullptr if is a leaf node.
  BVHNode *children_[2];

  /// Number of primitives in the leaf node. If 0, is an internal node.
  uint16_t numPrimitives_;

  /// Axis into which the node was split.
  uint16_t splitAxis_;

  /// Offset into the primitives array for the first primitive of the leaf node.
  uint32_t primitivesOffset_;
};

/**
 * A node of the BVH represented as an element in an array.
 * This struct has exactly 256bits, and is packed so that every component is
 * cache-aligned in the GPU for maximum performance.
 *
 * For an interior node, it's first child is always the next element in the
 * array, so the first child's index doesn't need to be stored, only the second
 * child's.
 */
struct LinearBVHNode {
  /// First point that represents the minimum of the bounding box.
  glm::vec3 min;

  /// Number of primitives in the node. If 0, the node is an interior node, and
  /// if > 0, the node is a leaf node.
  uint16_t numPrimitives;

  /// Coordinate axis the primitives were partitioned. This is used to traverse
  /// the tree in front-to-back order and skip bounding box intersections if a
  /// closer intersection has already been found. Only meaningful if the node is
  /// an interior node.
  uint16_t axis;

  /// Second point that represents the maximum of the bounding box.
  glm::vec3 max;

  union {
    /// If it's a leaf node, the offset into the primitives array.
    uint32_t primitivesOffset;

    /// If it's an internal node, the offset to the second child.
    uint32_t secondChildOffset;
  };
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_SCENE_BVH_HPP
