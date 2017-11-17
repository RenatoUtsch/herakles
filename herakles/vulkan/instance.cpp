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

#include "herakles/vulkan/instance.hpp"

#include <sstream>

#include <glog/logging.h>

#include "herakles/vulkan/internal/ext_loader.h"
#include "herakles/vulkan/utils.hpp"

namespace hk {
namespace {

/**
 * Returns the required instance extensions.
 * TODO(renatoutsch): make sure there are no repeated extensions, as that is an
 *   error in the Vulkan API usage.
 */
std::vector<const char *> getRequiredInstanceExtensions_(
    const SurfaceProvider &surfaceProvider,
    const std::vector<const char *> &extraExtensions,
    bool enableValidationLayers) {
  const auto &surfaceProviderExtensions =
      surfaceProvider.requiredInstanceExtensions();

  std::vector<const char *> extensions;
  extensions.insert(extensions.end(), surfaceProviderExtensions.begin(),
                    surfaceProviderExtensions.end());
  extensions.insert(extensions.end(), extraExtensions.begin(),
                    extraExtensions.end());

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  return extensions;
}

/**
 * Returns the required validation layers.
 * @param enableValidationLayers If validation layers will be enabled. If not,
 *   this function will return an empty vector.
 * @param extraValidationLayers Extra validation layers to add apart from
 *   standard validation.
 * @return The validation layers to enable.
 */
std::vector<const char *> getRequiredValidationLayers_(
    bool enableValidationLayers,
    const std::vector<const char *> &extraValidationLayers) {
  std::vector<const char *> validationLayers;

  if (enableValidationLayers) {
    validationLayers.push_back(DefaultValidationLayer);
    validationLayers.insert(validationLayers.end(),
                            extraValidationLayers.begin(),
                            extraValidationLayers.end());
  }

  return validationLayers;
}

/// Returns a vk::Instance from the input parameters.
vk::UniqueInstance createVkInstance_(
    const char *appName, const int appVersion, const char *engineName,
    const int engineVersion, const std::vector<const char *> &extensions,
    const std::vector<const char *> &validationLayers) {
  vk::ApplicationInfo appInfo;
  appInfo.setPApplicationName(appName)
      .setApplicationVersion(appVersion)
      .setPEngineName(engineName)
      .setEngineVersion(engineVersion)
      .setApiVersion(VK_API_VERSION_1_0);

  vk::InstanceCreateInfo createInfo;
  createInfo.setPApplicationInfo(&appInfo);

  if (!extensions.empty()) {
    createInfo.setEnabledExtensionCount(extensions.size())
        .setPpEnabledExtensionNames(extensions.data());
  }

  if (!validationLayers.empty()) {
    createInfo.setEnabledLayerCount(validationLayers.size())
        .setPpEnabledLayerNames(validationLayers.data());
  }

  return vk::createInstanceUnique(createInfo);
}

/// Debug report callback function. Logs with glog depending on severity.
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFunction_(
    VkDebugReportFlagsEXT flags,
    [[maybe_unused]] VkDebugReportObjectTypeEXT objType,
    [[maybe_unused]] uint64_t obj, [[maybe_unused]] size_t location,
    [[maybe_unused]] int32_t code, const char *layerPrefix, const char *msg,
    [[maybe_unused]] void *userData) {
  std::stringstream outputMessage;
  outputMessage << "Validation layer: " << layerPrefix << ": " << msg;

  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    LOG(ERROR) << outputMessage.str();
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    LOG(ERROR) << outputMessage.str();
  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    LOG(WARNING) << outputMessage.str();
  } else {
    VLOG(1) << outputMessage.str();
  }

  return VK_FALSE;
}

}  // namespace

vk::DebugReportFlagsEXT AllDebugReportFlags() {
  return vk::DebugReportFlagBitsEXT::eInformation |
         vk::DebugReportFlagBitsEXT::eWarning |
         vk::DebugReportFlagBitsEXT::ePerformanceWarning |
         vk::DebugReportFlagBitsEXT::eError |
         vk::DebugReportFlagBitsEXT::eDebug;
}

Instance::Instance(const char *appName, const uint32_t appVersion,
                   const bool enableValidationLayers,
                   const SurfaceProvider &surfaceProvider,
                   const vk::DebugReportFlagsEXT &debugReportFlags,
                   const std::vector<const char *> &extraExtensions,
                   const std::vector<const char *> &extraValidationLayers,
                   const char *engineName, const uint32_t engineVersion) {
  const std::vector<const char *> extensions = getRequiredInstanceExtensions_(
      surfaceProvider, extraExtensions, enableValidationLayers);
  validationLayersEnabled_ = enableValidationLayers,
  validationLayers_ = getRequiredValidationLayers_(enableValidationLayers,
                                                   extraValidationLayers);

  LOG(INFO) << "Creating instance";
  LOG(INFO) << "appName: " << appName;
  LOG(INFO) << "appVersion: " << appVersion;
  LOG(INFO) << "enableValidationLayers: " << enableValidationLayers;
  LOG(INFO) << "engineName: " << engineName;
  LOG(INFO) << "engineVersion: " << engineVersion;
  LOG(INFO) << "instanceExtensions: " << stringJoin(extensions, ", ");
  LOG(INFO) << "validationLayers: " << stringJoin(validationLayers_, ", ");

  vkInstance_ = createVkInstance_(appName, appVersion, engineName,
                                  engineVersion, extensions, validationLayers_);

  // Initialize *Ext functions for the instance.
  hkBindVulkanEXTFunctionsToInstance(*vkInstance_);

  if (enableValidationLayers) {
    setUpDebugReport_(debugReportFlags);
  }
}

void Instance::setUpDebugReport_(
    const vk::DebugReportFlagsEXT &debugReportFlags) {
  vk::DebugReportCallbackCreateInfoEXT createInfo;
  createInfo.setFlags(debugReportFlags)
      .setPfnCallback(debugReportCallbackFunction_);

  vkDebugReportCallback_ =
      vkInstance_->createDebugReportCallbackEXTUnique(createInfo);
}

}  // namespace hk
