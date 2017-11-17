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

#include "extensions.glsl"
#include "random.glsl"
#include "scene.glsl"

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
                       out vec3 n, out vec2 st) {
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
  n = normalize(cross(v0v1, v0v2));
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
bool intersectsScene(const Ray ray, const SkipTriangle skip,
                     out Interaction isect) {
  const vec3 invDir = 1.0f / ray.direction;
  const vec3 origByDir = ray.origin * invDir;
  const bvec3 dirIsNeg = bvec3(invDir.x < 0, invDir.y < 0, invDir.z < 0);

  bool hit = false;
  float t = INF;
  uint meshID = 0;
  uint begin = 0;
  vec3 n;
  vec2 st;

  uint nodesToVisit[64];
  int toVisitOffset = 0;
  nodesToVisit[0] = 0;

  float currT;
  vec2 currST;
  vec3 currN;
  uint numTriangles, splitAxis;
  while (toVisitOffset >= 0) {
    const uint currentNode = nodesToVisit[toVisitOffset--];
    const BVHNode node = BVHNodes[currentNode];
    unpackNumTrianglesAndAxis(node, numTriangles, splitAxis);

    if (intersectsBoundingBox(ray, t, node.minPoint, node.maxPoint, invDir,
                              origByDir)) {
      if (numTriangles == 0) {
        if (dirIsNeg[splitAxis]) {
          nodesToVisit[++toVisitOffset] = currentNode + 1;
          nodesToVisit[++toVisitOffset] = node.trianglesOrSecondChildOffset;
        } else {
          nodesToVisit[++toVisitOffset] = node.trianglesOrSecondChildOffset;
          nodesToVisit[++toVisitOffset] = currentNode + 1;
        }
      } else {
        for (int i = 0; i < numTriangles; ++i) {
          BVHTriangle triangle = BVHTriangles[node.trianglesOrSecondChildOffset
                                              + i];
          if (skip.skip && triangle.meshID == skip.meshID
              && triangle.begin == skip.begin) {
            continue;
          }
          if (intersectsTriangle(ray, triangle.begin, currT, currN, currST) &&
              currT <= t - EPSILON && currT > EPSILON) {
            hit = true;
            t = currT;
            meshID = triangle.meshID;
            begin = triangle.begin;
            n = currN;
            st = currST;
          }
        }
      }
    }
  }

  if (!hit) {
    return false;
  }

  // Shading normal disabled for now. Should not be used with Fresnel BSDFs.
  /* n = Normals[Indices[begin]] * st.s */
  /*   + Normals[Indices[begin + 1]] * st.t */
  /*   + Normals[Indices[begin + 2]] * (1.0f - st.s - st.t); */
  const bool backface = dot(-1.0f * ray.direction, n) < 0.0f;

  isect = Interaction(
      ray.origin + ray.direction * t,
      meshID,
      backface ? n * -1.0f : n,
      backface,
      begin);

  return true;
}

/// Tests if the ray is occluded by a shape in the given distance.
/// Stops if an intersection closer than the given point is found.
/// This is substantially faster than intersectsScene, so use it if you don't
/// need the interaction information. You can use minT = INF if you don't have
/// a minT.
bool unoccluded(const Ray ray, const float dist, const SkipTriangle skip) {
  const vec3 invDir = 1.0f / ray.direction;
  const vec3 origByDir = ray.origin * invDir;
  const bvec3 dirIsNeg = bvec3(invDir.x < 0, invDir.y < 0, invDir.z < 0);
  const float minT = dist - 1e-4; // To prevent hitting objects at exactly dist.

  uint nodesToVisit[64];
  int toVisitOffset = 0;
  nodesToVisit[0] = 0;

  float currT;
  vec2 currST;
  vec3 currN;
  uint numTriangles, splitAxis;
  while (toVisitOffset >= 0) {
    const uint currentNode = nodesToVisit[toVisitOffset--];
    const BVHNode node = BVHNodes[currentNode];
    unpackNumTrianglesAndAxis(node, numTriangles, splitAxis);

    if (intersectsBoundingBox(ray, minT, node.minPoint, node.maxPoint, invDir,
                              origByDir)) {
      if (numTriangles == 0) {
        if (dirIsNeg[splitAxis]) {
          nodesToVisit[++toVisitOffset] = currentNode + 1;
          nodesToVisit[++toVisitOffset] = node.trianglesOrSecondChildOffset;
        } else {
          nodesToVisit[++toVisitOffset] = node.trianglesOrSecondChildOffset;
          nodesToVisit[++toVisitOffset] = currentNode + 1;
        }
      } else {
        for (int i = 0; i < numTriangles; ++i) {
          BVHTriangle triangle = BVHTriangles[node.trianglesOrSecondChildOffset
                                              + i];
          if (skip.skip && triangle.meshID == skip.meshID
              && triangle.begin == skip.begin) {
            continue;
          }
          if (intersectsTriangle(ray, triangle.begin, currT, currN, currST) &&
              currT <= minT - EPSILON && currT > EPSILON) {
            return false;
          }
        }
      }
    }
  }

  return true;
}

#endif // !HERAKLES_SHADERS_INTERSECTION_GLSL
