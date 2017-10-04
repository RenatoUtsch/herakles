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

#ifndef HERAKLES_HERAKLES_VULKAN_CAMERA_HPP
#define HERAKLES_HERAKLES_VULKAN_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "herakles/vulkan/surface.hpp"

namespace hk {

/**
 * Struct that represents the camera in the same format used in GLSL.
 * This can be directly copied in UBOs and used with the GLSL camera functions.
 * The format of this struct might change with time, but the names of the
 * variables will not be changed.
 */
struct Camera {
  /// Position of the camera.
  // glm::vec3 position = {50, 52, 165.927};
  glm::vec3 position = {0, 1, 2.615};

  /// Field of view.
  float fov = 1.24f;
  // float fov = 0.34f;

  /// Direction of the camera.
  alignas(16) glm::vec3 direction = {0, 0, -1};
  // alignas(16) glm::vec3 direction = {0, 0, 1};

  /// Up vector of the camera.
  alignas(16) glm::vec3 up = {0, 1, 0};

  /// Right vector of the camera.
  alignas(16) glm::vec3 right = glm::cross(direction, up);
};

/**
 * Manages a Camera instance and automatically updates it's position.
 * Should only be used while the surface is alive, and only one camera manager
 * should be bound to one surface at a time.
 */
class CameraManager {
 public:
  /**
   * Initializes the camera manager for the given surface.
   * @param surface Surface where events will be captured for changing the
   *   camera parameters.
   * @param moveSpeed Speed at which the camera position changes.
   * @param mouseSpeed Speed at which the camera looks around.
   * @param horizontalAngle Initial horizontal angle, in radians.
   * @param verticalAngle Initial vertical angle, in radians.
   */
  CameraManager(const Surface &surface, float moveSpeed = 1.0f,
                float mouseSpeed = 0.05f,
                float horizontalAngle = glm::pi<float>(),
                float verticalAngle = 0.0f);

  /**
   * Updates the given camera with the latest changes in input.
   * This function should be called every frame.
   * @param camera The camera to be updated.
   * @param deltaTime How much time (in seconds) passed since the last time this
   *   function was called.
   * @return If there was any change in the camera position.
   */
  bool update(Camera &camera, float deltaTime);

 private:
  /// Updates the mouse. Returns if there was an update.
  bool mouseUpdate_(Camera &camera, float deltaTime);

  /// Updates the keyboard. Returns if there was an update.
  bool keyboardUpdate_(Camera &camera, float deltaTime);

  const Surface &surface_;
  float moveSpeed_;
  float mouseSpeed_;

  float horizontalAngle_;
  float verticalAngle_;
  double lastMouseX_;
  double lastMouseY_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_CAMERA_HPP
