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

#ifndef HERAKLES_HERAKLES_VULKAN_INSTANCE_HPP
#define HERAKLES_HERAKLES_VULKAN_INSTANCE_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/surface_provider.hpp"

namespace hk {

/// Name that identifies the Herakles engine.
inline const char *EngineName = "Herakles";

/// Version of the Herakles engine.
inline const uint32_t EngineVersion = VK_MAKE_VERSION(0, 0, 0);

/// The default validation layer.
inline const char *DefaultValidationLayer =
    "VK_LAYER_LUNARG_standard_validation";

/// Returns a flag that enables all debug report flags for the system.
vk::DebugReportFlagsEXT AllDebugReportFlags();

/**
 * Class responsible for managing a Vulkan instance.
 *
 * There should be only one Vulkan instance existing at one point in time.
 */
class Instance {
 public:
  /**
   * Initializes the Instance wrapper.
   *
   * When setting the extraExtensions variable, take care to not specify
   * extensions already specified by default. These are the
   * surfaceProvider.requiredExtensions() extensions and the
   * VK_EXT_DEBUG_REPORT_EXTENSION_NAME extension if enableValidationLayers is
   * set to true.
   *
   * When setting the extraValidationLayers, take care to not specify validation
   * layers already specified by default. The default validation layer is
   * DefaultValidationLayer.
   *
   * @param appName The name of the app.
   * @param appVersion The version of the app. Use VK_MAKE_VERSION for this.
   * @param enableValidationLayers Whether validation layers should be enabled.
   *   If set to true, will log the specified types of report with glog.
   * @param surfaceProvider Specifies where the surface used for rendering will
   *   come from. This provider should be the same provider specified when
   *   creating the surface.
   * @param debugReportFlags Flags that determine what kind of information will
   *   be logged when validation layers are enabled. Logs all reports by
   *   default, but only if enableValidationLayers is set to true.
   * @param extraExtensions Extra instance extensions to be enabled when
   *   creating the vulkan instance. This is an empty vector by default.
   * @param extraValidationLayers Extra validation layers to be enabled when
   *   creating the vulkan instance. If enableValidationLayers is not true, this
   *   argument is not used. This is an empty vector by default.
   * @param engineName Name of the instance's engine. Set to Herakles by
   *   default.
   * @param engineVersion Version of the instance's engine. Set to Herakles'
   *   version by default.
   */
  Instance(
      const char *appName, const uint32_t appVersion,
      const bool enableValidationLayers, const SurfaceProvider &surfaceProvider,
      const vk::DebugReportFlagsEXT &debugReportFlags = AllDebugReportFlags(),
      const std::vector<const char *> &extraExtensions = {},
      const std::vector<const char *> &extraValidationLayers = {},
      const char *engineName = EngineName,
      const uint32_t engineVersion = EngineVersion);

  /// Returns the vk::Instance managed by this class.
  const vk::Instance &vkInstance() const { return *vkInstance_; }

  /// Returns if validation layers are enabled.
  bool validationLayersEnabled() const { return validationLayersEnabled_; }

  /// Returns the enabled validation layers.
  const std::vector<const char *> &validationLayers() const {
    return validationLayers_;
  }

 private:
  /// Forwards vulkan debug report to glog.
  void setUpDebugReport_(const vk::DebugReportFlagsEXT &debugReportFlags);

  vk::UniqueInstance vkInstance_;
  vk::UniqueDebugReportCallbackEXT vkDebugReportCallback_;

  bool validationLayersEnabled_;
  std::vector<const char *> validationLayers_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_INSTANCE_HPP
