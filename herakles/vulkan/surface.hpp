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

#ifndef HERAKLES_HERAKLES_VULKAN_SURFACE_HPP
#define HERAKLES_HERAKLES_VULKAN_SURFACE_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

// GLFW needs to go after vulkan.
#include <GLFW/glfw3.h>

#include "herakles/vulkan/instance.hpp"

namespace hk {
/// Reprsents a GLFW window that will clean itself up on destruction.
using UniqueGLFWwindow =
    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;

/**
 * Represents a Vulkan surfaceKHR that is going to be used for presentation.
 */
class Surface {
 public:
  /**
   * Creates a surface.
   * @param surfaceProvider The surface provider for this surface.
   * @param instance The instance that the surface will be tied to.
   * @param title The title of the window.
   * @param requestedWidth The requested width of the window, if fullscreen is
   *   false. Please note that this is only a hint, and surfaces might have a
   *   different width compared to the specified width.
   * @param requestedHeight The requested height of the window, if fullscreen is
   *   false. Please note that this is only a hint, and surfaces might have a
   *   different height compared to the specified height.
   * @param fullscreen If true, the window will be fullscreen. If false, it will
   *   be windowed and use the width and height parameters.
   */
  Surface(const SurfaceProvider &surfaceProvider, const Instance &instance,
          const char *title, int requestedWidth, int requestedHeight,
          bool fullscreen);

  /**
   * Whether the user requested the program to exit.
   * @return true if the user requested the program to exit or false otherwise.
   */
  bool programShouldExit() const {
    return glfwWindowShouldClose(window_.get());
  }

  /**
   * Polls for OS events.
   * This notifies the operating system that the program is still responding.
   */
  void pollEvents() const { glfwPollEvents(); }

  /// Returns the required device extensions for using this surface.
  const std::vector<const char *> &requiredDeviceExtensions() const {
    return requiredDeviceExtensions_;
  }

  /**
   * Returns the requested width for the surface.
   * Please note that the real swapchain width might be different.
   */
  uint32_t requestedWidth() const { return requestedWidth_; }

  /**
   * Returns the requested height for the surface.
   * Please note that the real swapchain height might be different.
   */
  uint32_t requestedHeight() const { return requestedHeight_; }

  /// Returns the Vulkan vk::SurfaceKHR for this surface.
  const vk::SurfaceKHR &vkSurface() const { return *vkSurface_; };

  /// Returns the surface provider of this surface.
  const SurfaceProvider &surfaceProvider() const { return surfaceProvider_; }

  /// Returns the instance that manages this surface.
  const Instance &instance() const { return instance_; }

  /// Returns the GLFW window of this surface.
  GLFWwindow *window() const { return window_.get(); }

 private:
  const SurfaceProvider &surfaceProvider_;
  const Instance &instance_;
  std::vector<const char *> requiredDeviceExtensions_ = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  GLFWmonitor *monitor_;
  UniqueGLFWwindow window_;

  uint32_t requestedWidth_;
  uint32_t requestedHeight_;

  vk::SurfaceKHRDeleter vkSurfaceDeleter_;
  vk::UniqueSurfaceKHR vkSurface_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_SURFACE_HPP
