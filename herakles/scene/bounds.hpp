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

#ifndef HERAKLES_HERAKLES_SCENE_BOUNDS_HPP
#define HERAKLES_HERAKLES_SCENE_BOUNDS_HPP

#include <algorithm>

#include <glm/glm.hpp>

namespace hk {

/**
 * Represents an axis-aligned bounding box.
 */
template <typename T>
class Bounds3 {
 public:
  /// Minimum point of the bounding box.
  glm::vec3 minPoint;

  /// Maximum point of the bounding box.
  glm::vec3 maxPoint;

  /**
   * Initializes the bounding box with the two given points.
   */
  Bounds3(const glm::vec3 &p1, const glm::vec3 &p2)
      : minPoint(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                 std::min(p1.z, p2.z)),
        maxPoint(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                 std::max(p1.z, p2.z)) {}
};

/// Float version of the bounding box.
using Bounds3f = Bounds3<float>;

/**
 * An union between the two bounding boxes.
 */
template <typename T>
Bounds3<T> operator+(const Bounds3<T> &b1, const Bounds3<T> &b2) {
  return Bounds3<T>(glm::vec3(std::min(b1.minPoint.x, b2.minPoint.x),
                              std::min(b1.minPoint.y, b2.minPoint.y),
                              std::min(b1.minPoint.z, b2.minPoint.z)),
                    glm::vec3(std::max(b1.maxPoint.x, b2.maxPoint.x),
                              std::max(b1.maxPoint.y, b2.maxPoint.y),
                              std::max(b1.maxPoint.z, b2.maxPoint.z)));
}

/**
 * An union between the bounding box and another point.
 */
template <typename T>
Bounds3<T> operator+(const Bounds3<T> &b, const glm::vec3 &p) {
  return Bounds3<T>(
      glm::vec3(std::min(b.minPoint.x, p.x), std::min(b.minPoint.y, p.y),
                std::min(b.minPoint.z, p.z)),
      glm::vec3(std::max(b.maxPoint.x, p.x), std::max(b.maxPoint.y, p.y),
                std::max(b.maxPoint.z, p.z)));
}

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_SCENE_BOUNDS_HPP
