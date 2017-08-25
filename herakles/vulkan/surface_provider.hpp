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

#ifndef HERAKLES_HERAKLES_VULKAN_SURFACE_PROVIDER_HPP
#define HERAKLES_HERAKLES_VULKAN_SURFACE_PROVIDER_HPP

#include <vector>

namespace hk {

/**
 * Responsible for creating a surface given input specifications.
 *
 * This class must be kept alive for the entire life of all Surfaces created
 * from it. Only a single SurfaceProvider should exist per application.
 */
class SurfaceProvider {
 public:
  /**
   * Creates a surface provider.
   */
  SurfaceProvider();

  ~SurfaceProvider();

  /**
   * Returns the required instance extensions for creating a surface with this
   * provider.
   */
  const std::vector<const char *> &requiredInstanceExtensions() const {
    return requiredInstanceExtensions_;
  }

 private:
  // Builds the list of required instance extensions.
  void queryRequiredInstanceExtensions_();

  std::vector<const char *> requiredInstanceExtensions_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_SURFACE_PROVIDER_HPP
