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

#include "herakles/vulkan/surface.hpp"

#include <glog/logging.h>

namespace hk {

Surface::Surface(const SurfaceProvider &surfaceProvider,
                 const Instance &instance, const char *title,
                 int requestedWidth, int requestedHeight, bool fullscreen)
    : surfaceProvider_(surfaceProvider),
      instance_(instance),
      monitor_(nullptr),
      window_(nullptr, glfwDestroyWindow),
      requestedWidth_(requestedWidth),
      requestedHeight_(requestedHeight) {
  // GLFW is initialized on the SurfaceProvider.

  CHECK(requestedWidth > 0) << "Invalid GLFW window width";
  CHECK(requestedHeight > 0) << "Invalid GLFW window height";

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // No OpenGL, only Vulkan.

  if (fullscreen) {
    monitor_ = glfwGetPrimaryMonitor();
    CHECK(monitor_) << "Could not get the primary monitor";
  }

  GLFWwindow *window = glfwCreateWindow(requestedWidth, requestedHeight, title,
                                        monitor_, nullptr);
  CHECK(window) << "Failed to create a GLFW window";

  window_ = UniqueGLFWwindow(window, glfwDestroyWindow);

  // TODO(renatoutsch): set up callbacks for resize, user pointer and stuff.

  VkSurfaceKHR cVkSurface;
  VkResult err = glfwCreateWindowSurface(instance.vkInstance(), window_.get(),
                                         nullptr, &cVkSurface);
  CHECK(err == VK_SUCCESS) << "Failed to create a Vulkan surface in the window";

  vk::SurfaceKHRDeleter deleter(instance.vkInstance());
  vkSurface_ = vk::UniqueSurfaceKHR(cVkSurface, deleter);

  LOG(INFO) << "Surface created";
}

}  // namespace hk
