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

#include "bvh.hpp"

#include <array>
#include <memory>
#include <stack>

namespace {
using hk::BVHTriangle;
using hk::Bounds3f;
using hk::scene::Scene;

/**
 * Pointer-based representation of a node of the BVH.
 * Used to build the BVH, and later converted to the array representation.
 */
class BVHNode {
 public:
  /**
   * Builds an internal BVH node enclosing the two given leaf nodes.
   */
  BVHNode(std::unique_ptr<BVHNode> &&leaf1, std::unique_ptr<BVHNode> &&leaf2,
          uint16_t splitAxis)
      : bounds_(leaf1->bounds() + leaf2->bounds()),
        children_({{std::move(leaf1), std::move(leaf2)}}),
        numTriangles_(0),
        splitAxis_(splitAxis),
        trianglesOffset_(0) {}

  /**
   * Builds a leaf BVH node by specifying the enclosed triangles.
   */
  BVHNode(const Bounds3f &bounds, uint16_t numTriangles,
          uint32_t trianglesOffset)
      : bounds_(bounds),
        children_({{nullptr, nullptr}}),
        numTriangles_(numTriangles),
        splitAxis_(0),
        trianglesOffset_(trianglesOffset) {}

  /**
   * Returns the bounding box of the node.
   */
  const Bounds3f &bounds() const { return bounds_; }

 private:
  /// Bounding box of the node.
  Bounds3f bounds_;

  /// Children of the node. Nullptr if is a leaf node.
  std::array<std::unique_ptr<BVHNode>, 2> children_;

  /// Number of triangles in the leaf node. If 0, is an internal node.
  uint16_t numTriangles_;

  /// Axis into which the node was split.
  uint16_t splitAxis_;

  /// Offset into the triangles array for the first triangle of the leaf node.
  uint32_t trianglesOffset_;
};

/**
 * Information about the BVH triangles.
 */
struct BVHTriangleInfo {
  /// Index of the triangle in the triangles vector.
  size_t index;

  /// Bounding box of the triangle.
  Bounds3f bounds;

  /// Centroid of the triangle.
  glm::vec3 centroid;

  BVHTriangleInfo(size_t index, const Bounds3f &bounds)
      : index(index),
        bounds(bounds),
        centroid(bounds.minPoint * 0.5f + bounds.maxPoint * 0.5f) {}
};

/**
 * Builds the vector of triangles of the scene.
 */
std::vector<BVHTriangle> buildTriangles_(const Scene *scene) {
  std::vector<BVHTriangle> triangles;
  for (size_t i = 0; i < scene->meshes()->size(); ++i) {
    const auto *mesh = scene->meshes()->Get(i);
    for (size_t j = mesh->begin(); j < mesh->end(); j += 3) {
      triangles.emplace_back(i, j);
    }
  }

  return triangles;
}

/**
 * Converts a Flatbuffers vec4 point to a glm::vec3 point.
 */
glm::vec3 toVec3(const hk::scene::vec4 *vec) {
  return glm::vec3(vec->x(), vec->y(), vec->z());
}

/**
 * Returns the bounding box of a triangle.
 */
Bounds3f triangleBounds(const Scene *scene, const BVHTriangle &triangle) {
  const auto *p0 =
      scene->vertices()->Get(scene->indices()->Get(triangle.index));
  const auto *p1 =
      scene->vertices()->Get(scene->indices()->Get(triangle.index + 3));
  const auto *p2 =
      scene->vertices()->Get(scene->indices()->Get(triangle.index + 6));

  return Bounds3f(toVec3(p0), toVec3(p1)) + toVec3(p2);
}

/**
 * Returns a vector of BVHTriangleInfos, in the same order as the triangles.
 */
std::vector<BVHTriangleInfo> buildTriangleInfos_(
    const Scene *scene, const std::vector<BVHTriangle> &triangles) {
  std::vector<BVHTriangleInfo> triangleInfos;
  triangleInfos.reserve(triangles.size());

  for (size_t i = 0; i < triangles.size(); ++i) {
    triangleInfos.emplace_back(i, triangleBounds(scene, triangles[i]));
  }
  return triangleInfos;
}

/**
 * Builds a BVH tree using the Surface Area Heuristic.
 * @param triangles Triangles in the scene.
 * @param triangleInfos Vector of the same size of triangles
 */
std::pair<std::unique_ptr<BVHNode>, std::vector<BVHTriangle>> SAHBuild(
    const std::vector<BVHTriangle> &triangles,
    const std::vector<BVHTriangleInfo> &triangleInfos) {}

}  // namespace

namespace hk {

std::pair<std::vector<LinearBVHNode>, std::vector<BVHTriangle>> buildBVH(
    const Scene *scene) {
  const auto triangles = buildTriangles_(scene);
  const auto triangleInfos = buildTriangleInfos_(scene, triangles);

  const auto[root, orderedTriangles] = SAHBuild(triangles, triangleInfos);
  const auto linearBVH = linearizeBVH(root);

  return {linearBVH, orderedTriangles};
}

}  // namespace hk
