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

#include "herakles/vulkan/camera.hpp"

#include <GLFW/glfw3.h>

namespace hk {

CameraManager::CameraManager(const Surface &surface, float moveSpeed,
                             float mouseSpeed, float horizontalAngle,
                             float verticalAngle)
    : surface_(surface),
      moveSpeed_(moveSpeed),
      mouseSpeed_(mouseSpeed),
      horizontalAngle_(horizontalAngle),
      verticalAngle_(verticalAngle) {
  glfwSetInputMode(surface_.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwGetCursorPos(surface_.window(), &lastMouseX_, &lastMouseY_);
}

bool CameraManager::update(PinholeCamera &camera, float deltaTime) {
  bool changed = false;
  changed |= mouseUpdate_(camera, deltaTime);
  changed |= keyboardUpdate_(camera, deltaTime);
  return changed;
}

bool CameraManager::mouseUpdate_(PinholeCamera &camera, float deltaTime) {
  double xpos, ypos;
  glfwGetCursorPos(surface_.window(), &xpos, &ypos);
  if (glm::abs(lastMouseX_ - xpos) < glm::epsilon<double>() &&
      glm::abs(lastMouseY_ - ypos) < glm::epsilon<double>()) {
    return false;
  }

  horizontalAngle_ += (lastMouseX_ - xpos) * mouseSpeed_ * deltaTime;
  verticalAngle_ += (lastMouseY_ - ypos) * mouseSpeed_ * deltaTime;

  camera.direction = glm::normalize(
      glm::vec3(glm::cos(verticalAngle_) * glm::sin(horizontalAngle_),
                glm::sin(verticalAngle_),
                glm::cos(verticalAngle_) * glm::cos(horizontalAngle_)));

  camera.right = glm::normalize(
      glm::vec3(glm::sin(horizontalAngle_ - glm::half_pi<float>()), 0,
                glm::cos(horizontalAngle_ - glm::half_pi<float>())));

  camera.up = glm::cross(camera.right, camera.direction);

  lastMouseX_ = xpos;
  lastMouseY_ = ypos;
  return true;
}

bool CameraManager::keyboardUpdate_(PinholeCamera &camera, float deltaTime) {
  bool changed = false;

  if (glfwGetKey(surface_.window(), GLFW_KEY_UP) == GLFW_PRESS) {
    camera.position += camera.direction * deltaTime * moveSpeed_;
    changed = true;
  }
  if (glfwGetKey(surface_.window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.position -= camera.direction * deltaTime * moveSpeed_;
    changed = true;
  }
  if (glfwGetKey(surface_.window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.position += camera.right * deltaTime * moveSpeed_;
    changed = true;
  }
  if (glfwGetKey(surface_.window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.position -= camera.right * deltaTime * moveSpeed_;
    changed = true;
  }

  return changed;
}

}  // namespace hk
