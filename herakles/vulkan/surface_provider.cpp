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

#include "herakles/vulkan/surface_provider.hpp"

#include <memory>

#include <glog/logging.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

// GLFW needs to go after vulkan.
#include <GLFW/glfw3.h>

namespace hk {

SurfaceProvider::SurfaceProvider() {
  CHECK(glfwInit()) << "Failed to initialize GLFW";

  uint32_t count;
  const char **extensions = glfwGetRequiredInstanceExtensions(&count);
  CHECK(extensions) << "GLFW failed to query for required instance extensions";

  requiredInstanceExtensions_.resize(count);
  std::copy(extensions, extensions + count, requiredInstanceExtensions_.data());
}

SurfaceProvider::~SurfaceProvider() { glfwTerminate(); }

}  // namespace hk
