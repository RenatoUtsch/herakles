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
 * Sampling related functions.
 */

#ifndef HERAKLES_SHADERS_INTERSECTION_GLSL
#define HERAKLES_SHADERS_INTERSECTION_GLSL

#include "random.glsl"
#include "scene.glsl"

/// Spawns a ray from an intersection so that it won't be intersected again.
Ray spawnRay(const Interaction isect, const vec3 direction) {
  return Ray(isect.point + isect.normal * 1e-6, direction);
}

/// Computes the given triangle's area.
float triangleArea(const uint begin) {
  const vec3 v0 = Vertices[Indices[begin]];
  const vec3 v1 = Vertices[Indices[begin + 1]];
  const vec3 v2 = Vertices[Indices[begin + 2]];
  return 0.5 * length(cross(v1 - v0, v2 - v0));
}

/// Triangle intersection.
/// Normal is not normalized and not oriented.
bool intersectsTriangle(const Ray ray, const uint begin, out float t,
                       out vec2 st) {
  const vec3 v0 = Vertices[Indices[begin]];
  const vec3 v0v1 = Vertices[Indices[begin + 1]] - v0;
  const vec3 v0v2 = Vertices[Indices[begin + 2]] - v0;
  const vec3 pvec = cross(ray.direction, v0v2);
  const float det = dot(v0v1, pvec);

  // Ray and triangle are parallel if det is close to 0.
  if (abs(det) < EPSILON) return false;

  const float invDet = 1.0f / det;
  const vec3 tvec = ray.origin - v0;
  st.s = dot(tvec, pvec) * invDet;
  if (st.s <= -EPSILON || st.s >= 1.0f + EPSILON) {
    return false;
  }

  const vec3 qvec = cross(tvec, v0v1);
  st.t = dot(ray.direction, qvec) * invDet;
  if (st.t <= -EPSILON || st.s + st.t >= 1.0f + EPSILON) {
    return false;
  }

  t = dot(v0v2, qvec) * invDet;
  return true;
}

/*
 * Returns if the given bounding box is intersected by the given ray.
 */
bool intersectsBoundingBox(
    const Ray ray, const float rayTMax, const vec3 minPoint, const vec3 maxPoint,
    const vec3 invDir, const vec3 origByDir) {
  const vec3 p0 = minPoint * invDir - origByDir;
  const vec3 p1 = maxPoint * invDir - origByDir;
  const vec3 pMin = min(p0, p1);
  const vec3 pMax = max(p0, p1);

  float tMin = max(max(max(0.0f, pMin.x), pMin.y), pMin.z);
  float tMax = min(min(min(rayTMax, pMax.x), pMax.y), pMax.z);

  // Update tMax to ensure robust bounds intersection.
  tMax *= 1.0f + 2.0f * GAMMA_3;

  return tMin <= tMax;
}

/// Ray-scene intersection.
/// Returns the interaction at intersection point.
bool intersectsScene(const Ray ray, out Interaction isect) {
  const vec3 invDir = 1.0f / ray.direction;
  const vec3 origByDir = ray.origin * invDir;
  const bvec3 dirIsNeg = bvec3(invDir.x < 0, invDir.y < 0, invDir.z < 0);

  bool hit = false;
  float t = INF;
  uint meshID = 0;
  uint begin = 0;
  vec2 st;

  uint nodesToVisit[64];
  nodesToVisit[0] = 0;

  float currT;
  vec2 currST;

  int toVisitOffset = 0;
  uint currentNode = nodesToVisit[toVisitOffset];
  BVHNode node = BVHNodes[currentNode];
  uint numTriangles, splitAxis;
  unpackNumTrianglesAndAxis(node, numTriangles, splitAxis);
  while (toVisitOffset >= 0) {
    if (intersectsBoundingBox(ray, t, node.minPoint, node.maxPoint, invDir,
                              origByDir)) {
      if (numTriangles == 0) {
        if (dirIsNeg[splitAxis]) {
          nodesToVisit[toVisitOffset++] = currentNode + 1;
          nodesToVisit[toVisitOffset++] = node.trianglesOrSecondChildOffset;
        } else {
          nodesToVisit[toVisitOffset++] = node.trianglesOrSecondChildOffset;
          nodesToVisit[toVisitOffset++] = currentNode + 1;
        }
      } else {
        for (int i = 0; i < numTriangles; ++i) {
          BVHTriangle triangle = BVHTriangles[node.trianglesOrSecondChildOffset
                                              + i];
          if (intersectsTriangle(ray, triangle.begin, currT, currST) &&
              currT <= t - EPSILON && currT > EPSILON) {
            hit = true;
            t = currT;
            meshID = triangle.meshID;
            begin = triangle.begin;
            st = currST;
          }
        }
      }
    }

    if (--toVisitOffset < 0) {
      break;
    }
    currentNode = nodesToVisit[toVisitOffset];
    node = BVHNodes[currentNode];
    unpackNumTrianglesAndAxis(node, numTriangles, splitAxis);
  }

  if (!hit) {
    return false;
  }

  const vec3 normal = Normals[Indices[begin]] * st.s
                    + Normals[Indices[begin + 1]] * st.t
                    + Normals[Indices[begin + 2]] * (1.0f - st.s - st.t);
  const bool backface = dot(normal, ray.direction) <= -EPSILON ? false : true;

  isect = Interaction(
      ray.origin + ray.direction * t,
      meshID,
      backface ? normal * -1.0f : normal,
      backface);

  return true;
}

/// Tests if the ray is occluded by a shape in the given distance.
/// Stops if an intersection closer than the given point is found.
/// This is substantially faster than intersectsScene, so use it if you don't
/// need the interaction information. You can use minT = INF if you don't have
/// a minT.
bool unoccluded(const Ray ray, const float dist) {
  const vec3 invDir = 1.0f / ray.direction;
  const vec3 origByDir = ray.origin * invDir;
  const bvec3 dirIsNeg = bvec3(invDir.x < 0, invDir.y < 0, invDir.z < 0);
  const float minT = dist - 1e-4; // To prevent hitting objects at exactly dist.
  float currT;
  vec2 currST;

  uint toVisitOffset = 0, currentNodeIndex = 0;
  uint nodesToVisit[64];
  while (true) {
    const BVHNode node = BVHNodes[currentNodeIndex];
    uint numTriangles, splitAxis;
    unpackNumTrianglesAndAxis(node, numTriangles, splitAxis);

    // Check ray against BVH node.
    if (intersectsBoundingBox(ray, minT, node.minPoint, node.maxPoint, invDir,
                              origByDir)) {
      if (numTriangles > 0) {
        for (int i = 0; i < numTriangles; ++i) {
          BVHTriangle triangle = BVHTriangles[node.trianglesOrSecondChildOffset
                                              + i];
          if (intersectsTriangle(ray, triangle.begin, currT, currST) &&
              currT <= minT - EPSILON && currT > EPSILON) {
            return false;
          }
        }

        if (toVisitOffset == 0) break;
        currentNodeIndex = nodesToVisit[--toVisitOffset];
      } else {
        if (dirIsNeg[splitAxis]) {
          nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
          currentNodeIndex = node.trianglesOrSecondChildOffset;
        } else {
          nodesToVisit[toVisitOffset++] = node.trianglesOrSecondChildOffset;
          currentNodeIndex = currentNodeIndex + 1;
        }
      }
    } else {
      if (toVisitOffset == 0) break;
      currentNodeIndex = nodesToVisit[--toVisitOffset];
    }
  }
  return true;
}

#endif // !HERAKLES_SHADERS_INTERSECTION_GLSL
