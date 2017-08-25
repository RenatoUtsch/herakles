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

#include "herakles/vulkan/physical_device.hpp"

#include <set>

#include <glog/logging.h>

namespace hk {

PhysicalDevice::PhysicalDevice(vk::PhysicalDevice &&vkPhysicalDevice,
                               const Surface &surface,
                               const std::vector<const char *> &extraExtensions)
    : vkPhysicalDevice_(vkPhysicalDevice) {
  vkPhysicalDeviceProperties_ = vkPhysicalDevice_.getProperties();
  vkQueueFamilyProperties_ = vkPhysicalDevice_.getQueueFamilyProperties();

  LOG(INFO) << "Device: " << vkPhysicalDeviceProperties_.deviceName;

  chooseComputeQueueFamily_(surface);
  chooseTransferQueueFamily_();
  choosePresentationQueueFamily_(surface);

  const auto &surfaceExtensions = surface.requiredDeviceExtensions();
  requiredDeviceExtensions_.insert(requiredDeviceExtensions_.end(),
                                   surfaceExtensions.begin(),
                                   surfaceExtensions.end());
  requiredDeviceExtensions_.insert(requiredDeviceExtensions_.end(),
                                   extraExtensions.begin(),
                                   extraExtensions.end());

  checkForExtensionSupport_();
  checkForSwapchainSupport_(surface);

  saveQueueFamilyIndices_();

  // If the physical device is constructed successfully, it is suitable for
  // Herakles.
  LOG(INFO) << "Device suitable for Herakles";
}

void PhysicalDevice::chooseComputeQueueFamily_(const Surface &surface) {
  uint32_t queueWithoutPresentationCount = 0;
  uint32_t queueWithPresentationCount = 0;
  uint32_t queueWithoutPresentationIndex;
  uint32_t queueWithPresentationIndex;

  for (uint32_t i = 0; i < vkQueueFamilyProperties_.size(); ++i) {
    const auto &queueFamily = vkQueueFamilyProperties_[i];
    if (!queueFamily.queueCount) continue;

    if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
      if (/* TODO(renatoutsch): surface.requiresPresentationSupport() && */
          queueFamilySupportsPresentation_(i, surface) &&
          queueFamily.queueCount > queueWithPresentationCount) {
        queueWithPresentationCount = queueFamily.queueCount;
        queueWithPresentationIndex = i;
      } else if (queueFamily.queueCount > queueWithoutPresentationCount) {
        queueWithoutPresentationCount = queueFamily.queueCount;
        queueWithoutPresentationIndex = i;
      }
    }
  }

  if (queueWithPresentationCount) {
    LOG(INFO) << "Compute queue family is the same "
              << "as the presentation queue family";
    computeQueueFamilyIndex_ = queueWithPresentationIndex;
    presentationQueueFamilyIndex_ = queueWithPresentationIndex;
    hasPresentationQueueFamilyIndex_ = true;
  } else if (queueWithoutPresentationCount) {
    if (true /* TODO(renatoutsch): surface.requiresPresentationSupport() */) {
      LOG(INFO) << "Compute queue family is separate "
                << "from the presentation queue";
    }
    computeQueueFamilyIndex_ = queueWithoutPresentationIndex;
  } else {
    throw error::NoSuitableQueuesFound("No appropriate compute queue family.");
  }

  LOG(INFO) << "Compute queue family index: " << computeQueueFamilyIndex_;
}

void PhysicalDevice::chooseTransferQueueFamily_() {
  uint32_t generalQueueCount = 0;
  uint32_t specificQueueCount = 0;
  uint32_t generalQueueFamilyIndex;
  uint32_t specificQueueFamilyIndex;
  for (uint32_t i = 0; i < vkQueueFamilyProperties_.size(); ++i) {
    const auto &queueFamily = vkQueueFamilyProperties_[i];

    // Skip this queue family if it is already reserved for compute.
    if (!queueFamily.queueCount || i == computeQueueFamilyIndex_) continue;

    // Try to select a queue different from the one already selected for
    // compute.
    if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute &&
        queueFamily.queueCount > generalQueueCount) {
      generalQueueCount = queueFamily.queueCount;
      generalQueueFamilyIndex = i;
    }

    // Try to find a queue that is specific for transfers.
    if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
      if (queueFamily.queueCount > generalQueueCount) {
        generalQueueCount = queueFamily.queueCount;
        generalQueueFamilyIndex = i;
      }

      if (!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
          queueFamily.queueCount > specificQueueCount) {
        specificQueueCount = queueFamily.queueCount;
        specificQueueFamilyIndex = i;
      }
    }
  }

  if (specificQueueCount) {
    LOG(INFO) << "Transfer queue family is specific for transfers";
    transferQueueFamilyIndex_ = specificQueueFamilyIndex;
  } else if (generalQueueCount) {
    LOG(INFO) << "Transfer queue family is a separate general family";
    transferQueueFamilyIndex_ = generalQueueFamilyIndex;
  } else {
    // Fallback to the compute queue family, as it also supports transfer
    // operations.
    LOG(INFO) << "Transfer queue family is the compute queue family";
    transferQueueFamilyIndex_ = computeQueueFamilyIndex_;
  }

  LOG(INFO) << "Transfer queue family index: " << transferQueueFamilyIndex_;
}

void PhysicalDevice::choosePresentationQueueFamily_(const Surface &surface) {
  if (false /* TODO(renatoutsch): !surface.requiresPresentationSupport() */) {
    LOG(INFO) << "Presentation queue family not required";
    return;
  }
  if (hasPresentationQueueFamilyIndex_) {
    LOG(INFO) << "Presentation queue family index: "
              << presentationQueueFamilyIndex_;
    return;
  }

  bool transferQueueFamilySupportsPresentation = false;
  uint32_t queueCount = 0;
  uint32_t queueFamilyIndex;
  for (uint32_t i = 0; i < vkQueueFamilyProperties_.size(); ++i) {
    const auto &queueFamily = vkQueueFamilyProperties_[i];

    // Skip this queue family if it is already reserved for compute or transfer.
    if (!queueFamily.queueCount || i == computeQueueFamilyIndex_) continue;

    if (queueFamilySupportsPresentation_(i, surface)) {
      if (i == transferQueueFamilyIndex_) {
        transferQueueFamilySupportsPresentation = true;
      } else if (queueFamily.queueCount > queueCount) {
        queueCount = queueFamily.queueCount;
        queueFamilyIndex = i;
      }
    }
  }

  if (queueCount) {
    LOG(INFO) << "Presentation queue family is separate";
    hasPresentationQueueFamilyIndex_ = true;
    presentationQueueFamilyIndex_ = queueFamilyIndex;
  } else if (transferQueueFamilySupportsPresentation) {
    // Not ideal, but it works.
    LOG(INFO) << "Presentation queue family is the transfer queue family";
    hasPresentationQueueFamilyIndex_ = true;
    presentationQueueFamilyIndex_ = transferQueueFamilyIndex_;
  } else {
    throw error::NoSuitableQueuesFound(
        "No queue family with presentation support.");
  }

  LOG(INFO) << "Presentation queue family index: "
            << presentationQueueFamilyIndex_;
}

bool PhysicalDevice::queueFamilySupportsPresentation_(uint32_t queueFamilyIndex,
                                                      const Surface &surface) {
  if (!surface.vkSurface()) return false;

  return vkPhysicalDevice_.getSurfaceSupportKHR(queueFamilyIndex,
                                                surface.vkSurface());
}

void PhysicalDevice::checkForExtensionSupport_() {
  vkDeviceExtensionProperties_ =
      vkPhysicalDevice_.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensionSet(requiredDeviceExtensions_.begin(),
                                             requiredDeviceExtensions_.end());

  for (const auto &extension : vkDeviceExtensionProperties_) {
    requiredExtensionSet.erase(extension.extensionName);
  }

  // If all extensions are supported, requiredExtensionSet will now be empty.
  CHECK(requiredExtensionSet.empty())
      << "Not all device extensions are supported.";
}

void PhysicalDevice::checkForSwapchainSupport_(const Surface &surface) {
  if (false /* TODO(renatoutsch): !surface.requiresPresentationSupport() */)
    return;

  vkSurfaceCapabilities_ =
      vkPhysicalDevice_.getSurfaceCapabilitiesKHR(surface.vkSurface());
  vkSurfaceFormats_ =
      vkPhysicalDevice_.getSurfaceFormatsKHR(surface.vkSurface());
  vkSurfacePresentModes_ =
      vkPhysicalDevice_.getSurfacePresentModesKHR(surface.vkSurface());

  CHECK(!vkSurfaceFormats_.empty())
      << "Swapchain not suitable, has no surface formats.";
  CHECK(!vkSurfacePresentModes_.empty())
      << "Swapchain not suitable, has no present modes.";
}

void PhysicalDevice::saveQueueFamilyIndices_() {
  std::set<uint32_t> indices = {computeQueueFamilyIndex_,
                                transferQueueFamilyIndex_};

  if (supportsPresentation()) {
    indices.insert(presentationQueueFamilyIndex_);
  }

  queueFamilyIndices_.insert(queueFamilyIndices_.end(), indices.begin(),
                             indices.end());
}

PhysicalDevice pickPhysicalDevice(const Instance &instance,
                                  const Surface &surface) {
  LOG(INFO) << "Picking a physical device";

  auto vkPhysicalDevices = instance.vkInstance().enumeratePhysicalDevices();
  for (auto &vkPhysicalDevice : vkPhysicalDevices) {
    try {
      return PhysicalDevice(std::move(vkPhysicalDevice), surface);
    } catch (const error::NoSuitableQueuesFound &e) {
      LOG(INFO) << e.what();  // Info because other device may succeed.
    }

    LOG(INFO) << "Physical device unsuitable. Trying another...";
  }

  LOG(ERROR) << "None of the physical devices were suitable.";
  throw error::NoSuitablePhysicalDeviceFound(
      "None of the devices were suitable.");
}

}  // namespace hk
