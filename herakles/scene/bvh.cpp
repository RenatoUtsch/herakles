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

#include <algorithm>
#include <array>
#include <memory>
#include <stack>
#include <tuple>

#include <glog/logging.h>

namespace {
using hk::BVHNode;
using hk::BVHTriangle;
using hk::Bounds3f;
using hk::scene::Scene;

/**
 * Pointer-based representation of a node of the BVH.
 * Used to build the BVH, and later converted to the array representation.
 */
struct BVHBuildNode {
  /// Bounding box of the node.
  Bounds3f bounds;

  /// Children of the node. Nullptr if is a leaf node.
  std::array<std::unique_ptr<BVHBuildNode>, 2> children;

  /// Number of triangles in the leaf node. If 0, is an internal node.
  uint16_t numTriangles;

  /// Axis into which the node was split.
  uint16_t splitAxis;

  /// Offset into the triangles array for the first triangle of the leaf node.
  uint32_t trianglesOffset;

  /**
   * Builds an internal BVH node enclosing the two given leaf nodes.
   */
  BVHBuildNode(uint16_t splitAxis, std::unique_ptr<BVHBuildNode> &&leaf1,
               std::unique_ptr<BVHBuildNode> &&leaf2)
      : bounds(leaf1->bounds + leaf2->bounds),
        children({{std::move(leaf1), std::move(leaf2)}}),
        numTriangles(0),
        splitAxis(splitAxis),
        trianglesOffset(0) {}

  /**
   * Builds a leaf BVH node by specifying the enclosed triangles.
   */
  BVHBuildNode(const Bounds3f &bounds, uint16_t numTriangles,
               uint32_t trianglesOffset)
      : bounds(bounds),
        children({{nullptr, nullptr}}),
        numTriangles(numTriangles),
        splitAxis(0),
        trianglesOffset(trianglesOffset) {}
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
glm::vec3 toVec3_(const hk::scene::vec4 *vec) {
  return glm::vec3(vec->x(), vec->y(), vec->z());
}

/**
 * Returns the bounding box of a triangle.
 */
Bounds3f triangleBounds_(const Scene *scene, const BVHTriangle &triangle) {
  const auto *p0 =
      scene->vertices()->Get(scene->indices()->Get(triangle.begin));
  const auto *p1 =
      scene->vertices()->Get(scene->indices()->Get(triangle.begin + 1));
  const auto *p2 =
      scene->vertices()->Get(scene->indices()->Get(triangle.begin + 2));

  return Bounds3f(toVec3_(p0), toVec3_(p1)) + toVec3_(p2);
}

/**
 * Returns a vector of BVHTriangleInfos, in the same order as the triangles.
 */
std::vector<BVHTriangleInfo> buildTriangleInfos_(
    const Scene *scene, const std::vector<BVHTriangle> &triangles) {
  std::vector<BVHTriangleInfo> triangleInfos;
  triangleInfos.reserve(triangles.size());

  for (size_t i = 0; i < triangles.size(); ++i) {
    triangleInfos.emplace_back(i, triangleBounds_(scene, triangles[i]));
  }
  return triangleInfos;
}

/**
 * Builds a leaf node with the given data.
 */
std::unique_ptr<BVHBuildNode> buildLeafNode(
    const std::vector<BVHTriangle> &triangles,
    const std::vector<BVHTriangleInfo> &triangleInfos, const Bounds3f &bounds,
    size_t start, size_t end, size_t numTriangles,
    std::vector<BVHTriangle> &orderedTriangles) {
  const size_t triangleOffset = orderedTriangles.size();
  for (size_t i = start; i < end; ++i) {
    orderedTriangles.push_back(triangles[triangleInfos[i].index]);
  }

  return std::make_unique<BVHBuildNode>(bounds, numTriangles,
                                        (uint32_t)triangleOffset);
}

/**
 * Partitions BVH primitives into equally-sized subsets.
 * @param dim The dimension along which triangles are being partitioned.
 * @param start Start of the range to split.
 * @param end One after end of the range to split.
 * @param triangleInfos Vector to be partitioned in-place.
 * Returns the split point.
 */
size_t equallySizedSplit_(int dim, size_t start, size_t end,
                          std::vector<BVHTriangleInfo> &triangleInfos) {
  // Partition primitives using equally-sized subsets.
  const size_t mid = (start + end) / 2;
  std::nth_element(&triangleInfos[start], &triangleInfos[mid],
                   &triangleInfos[end - 1] + 1,
                   [dim](const BVHTriangleInfo &a, const BVHTriangleInfo &b) {
                     return a.centroid[dim] < b.centroid[dim];
                   });
  return mid;
}

// SAH constants.
constexpr size_t NumBuckets = 12;
constexpr size_t MaxTrianglesInNode = 1;
const float IntersectionCost = 1.0f;
const float TraversalCost = 1.0f;

/**
 * Buckets for approximate SAH.
 */
struct BucketInfo {
  int count = 0;
  Bounds3f bounds;
};

/**
 * Partitions BVH primitives by following the Surface Area Heuristic (SAH).
 * @param dim The dimension along which triangles are being partitioned.
 * @param start Start of the range to split.
 * @param end One after end of the range to split.
 * @param numTriangles Number of triangles in the range.
 * @param bounds Bounds of all primitives in the node.
 * @param centroidBounds Bounds of the centroids.
 * @param triangleInfos Vector to be partitioned in-place.
 * @return the split point, or 0 if is to just create a leaf node with the
 * node's triangles.
 */
size_t sahSplit_(int dim, size_t start, size_t end, size_t numTriangles,
                 const Bounds3f &bounds, const Bounds3f &centroidBounds,
                 std::vector<BVHTriangleInfo> &triangleInfos) {
  // Initialize buckets.
  BucketInfo buckets[NumBuckets];
  for (size_t i = start; i < end; ++i) {
    size_t b =
        NumBuckets * centroidBounds.offset(triangleInfos[i].centroid)[dim];
    if (b == NumBuckets) b = NumBuckets - 1;
    CHECK_GE(b, 0);
    CHECK_LT(b, NumBuckets);

    ++buckets[b].count;
    buckets[b].bounds += triangleInfos[i].bounds;
  }

  // Compute cost for splitting after each bucket.
  float cost[NumBuckets - 1];
  for (size_t i = 0; i < NumBuckets - 1; ++i) {
    Bounds3f b0, b1;
    int count0 = 0, count1 = 1;
    for (size_t j = 0; j <= i; ++j) {
      b0 += buckets[j].bounds;
      count0 += buckets[j].count;
    }
    for (size_t j = i + 1; j < NumBuckets; ++j) {
      b1 += buckets[j].bounds;
      count1 += buckets[j].count;
    }

    cost[i] = TraversalCost +
              (count0 * b0.surfaceArea() + count1 * b1.surfaceArea()) /
                  bounds.surfaceArea();
  }

  // Find bucket to split that minimizes SAH metric.
  float minCost = cost[0];
  size_t minCostSplitBucket = 0;
  for (size_t i = 1; i < NumBuckets - 1; ++i) {
    if (cost[i] < minCost) {
      minCost = cost[i];
      minCostSplitBucket = i;
    }
  }

  // Either create leaf ndoe or split primitives at selected SAH bucket.
  const float leafCost = IntersectionCost * numTriangles;
  if (numTriangles > MaxTrianglesInNode || minCost < leafCost) {
    BVHTriangleInfo *pMid = std::partition(
        &triangleInfos[start], &triangleInfos[end - 1] + 1,
        [=](const BVHTriangleInfo &pi) {
          size_t b = NumBuckets * centroidBounds.offset(pi.centroid)[dim];
          if (b == NumBuckets) b = NumBuckets - 1;
          CHECK_GE(b, 0);
          CHECK_LT(b, NumBuckets);
          return b <= minCostSplitBucket;
        });
    return pMid - &triangleInfos[0];
  }

  return 0;
}

/**
 * Builds a BVH tree using the Surface Area Heuristic.
 * @param triangles Triangles in the scene.
 * @param triangleInfos Vector of the same size of triangles
 */
std::unique_ptr<BVHBuildNode> SAHBuild_(
    const std::vector<BVHTriangle> &triangles,
    std::vector<BVHTriangleInfo> &triangleInfos, size_t start, size_t end,
    size_t &totalNodes, std::vector<BVHTriangle> &orderedTriangles) {
  CHECK_LT(start, end);
  ++totalNodes;

  // Bounds of all primitives in the BVH node.
  Bounds3f bounds;
  for (size_t i = start; i < end; ++i) {
    bounds += triangleInfos[i].bounds;
  }

  // If only one triangle, return a leaf node.
  const size_t numTriangles = end - start;
  if (numTriangles == 1) {
    return buildLeafNode(triangles, triangleInfos, bounds, start, end,
                         numTriangles, orderedTriangles);
  }

  // Compute bound of primitive centroids, choose split dimension.
  Bounds3f centroidBounds;
  for (size_t i = start; i < end; ++i) {
    centroidBounds += triangleInfos[i].centroid;
  }
  const int dim = centroidBounds.maximumExtentAxis();

  // If centroids are on the same position, return a leaf node.
  // Partitioning further doesn't produce good results.
  if (centroidBounds.maxPoint[dim] == centroidBounds.minPoint[dim]) {
    return buildLeafNode(triangles, triangleInfos, bounds, start, end,
                         numTriangles, orderedTriangles);
  }

  size_t splitPoint;
  if (numTriangles <= 2) {
    splitPoint = equallySizedSplit_(dim, start, end, triangleInfos);
  } else {
    splitPoint = sahSplit_(dim, start, end, numTriangles, bounds,
                           centroidBounds, triangleInfos);
    if (!splitPoint) {
      return buildLeafNode(triangles, triangleInfos, bounds, start, end,
                           numTriangles, orderedTriangles);
    }
  }

  return std::make_unique<BVHBuildNode>(
      dim,
      SAHBuild_(triangles, triangleInfos, start, splitPoint, totalNodes,
                orderedTriangles),
      SAHBuild_(triangles, triangleInfos, splitPoint, end, totalNodes,
                orderedTriangles));
}

/**
 * Flattens the BVH so that it can be uploaded to the GPU.
 */
std::vector<BVHNode> flattenBVH_(const BVHBuildNode &root, size_t numNodes) {
  std::vector<BVHNode> nodes(numNodes);
  std::stack<std::tuple<const BVHBuildNode &, BVHNode *>> s;

  s.emplace(root, nullptr);
  for (size_t offset = 0; !s.empty(); ++offset) {
    const auto[node, parentPtr] = s.top();
    s.pop();

    auto &linearNode = nodes[offset];
    linearNode.minPoint = node.bounds.minPoint;
    linearNode.maxPoint = node.bounds.maxPoint;
    linearNode.numTriangles = node.numTriangles;

    if (node.numTriangles > 0) {
      linearNode.trianglesOffset = node.trianglesOffset;
    } else {
      linearNode.splitAxis = node.splitAxis;

      // Add second child first and first child afterwards, because first child
      // will go right after the parent in the linear BVH.
      s.emplace(*node.children[1], &linearNode);
      s.emplace(*node.children[0], nullptr);
    }

    // If parentPtr is available, we're the second child. Save our offset.
    if (parentPtr) {
      parentPtr->secondChildOffset = offset;
    }
  }

  return nodes;
}

}  // namespace

std::ostream &operator<<(std::ostream &out, const glm::vec3 &v) {
  out << "[" << v.x << ", " << v.y << ", " << v.z << "]";
  return out;
}

namespace hk {

BVHData buildBVH(const Scene *scene) {
  const auto triangles = buildTriangles_(scene);
  auto triangleInfos = buildTriangleInfos_(scene, triangles);

  std::vector<BVHTriangle> orderedTriangles;
  orderedTriangles.reserve(triangles.size());

  size_t totalNodes = 0;
  const auto root = SAHBuild_(triangles, triangleInfos, 0, triangles.size(),
                              totalNodes, orderedTriangles);
  auto flattenedBVH = flattenBVH_(*root, totalNodes);

  return {std::move(flattenedBVH), std::move(orderedTriangles)};
}

}  // namespace hk
