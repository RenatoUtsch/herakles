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

#ifndef HERAKLES_HERAKLES_PHYSICAL_DEVICE_HPP
#define HERAKLES_HERAKLES_PHYSICAL_DEVICE_HPP

#include <set>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/instance.hpp"
#include "herakles/vulkan/surface.hpp"

namespace hk {
namespace error {

/**
 * Thrown when not all required extensions are supported.
 */
class DeviceExtensionsNotSupported : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * Thrown when a physical device doesn't support presentation and it's required.
 */
class NoPresentationSupport : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * Thrown when the physical device being constructed has no suitable queues.
 */
class NoSuitableQueuesFound : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * Thrown when a suitable physical device is not found.
 */
class NoSuitablePhysicalDeviceFound : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

}  // namespace error

/**
 * Represents a single physical device and the information it provides.
 *
 * The information provided for the physical device is linked to the instance of
 * the Surface provided to the constructor, as presentation/swapchain
 * information is specific to a surface.
 *
 * The selected queue families are the most specific queues for each kind of
 * capability.
 */
class PhysicalDevice {
 public:
  /**
   * Constructs the physical device wrapper from a vk::PhysicalDevice instance.
   * @param vkPhysicalDevice The physical device to be wrapped by this class.
   * @param Surface The surface that is going to be the render target of the
   *   physical device.
   * @param extraExtensions Required extra extensions apart from the ones
   *   required by the surface.
   */
  PhysicalDevice(vk::PhysicalDevice &&vkPhysicalDevice, const Surface &surface,
                 const std::vector<const char *> &extraExtensions = {});

  /// Returns the Vulkan physical device.
  const vk::PhysicalDevice &vkPhysicalDevice() const {
    return vkPhysicalDevice_;
  }

  /// Returns the index of the preferred compute queue family.
  uint32_t computeQueueFamilyIndex() const { return computeQueueFamilyIndex_; }

  /**
   * Returns the index of the preferred transfer queue family.
   * Please note that this might not be the same queue family as the compute
   * queue family.
   */
  uint32_t transferQueueFamilyIndex() const {
    return transferQueueFamilyIndex_;
  }

  /// Returns whether presentation is supported by this physical device.
  bool supportsPresentation() const { return hasPresentationQueueFamilyIndex_; }

  /**
   * Returns the index of the preferred presentation queue family index.
   * Please note that this might not be the same queue family as the compute
   * queue family.
   * Only call this function if supportsPresentation() returns true.
   * @return The presentation queue family index.
   * @throws NoPresentationSupport If presentation is not supported.
   */
  uint32_t presentationQueueFamilyIndex() const {
    if (supportsPresentation()) {
      return presentationQueueFamilyIndex_;
    }
    throw error::NoPresentationSupport(
        "presentationQueueFamilyIndex() not available.");
  }

  /// Returns the selected queue family indices, without repeated entries.
  const std::vector<uint32_t> &queueFamilyIndices() const {
    return queueFamilyIndices_;
  }

  /// Returns the extensions to be enabled when creating the logical device.
  const std::vector<const char *> &requiredDeviceExtensions() const {
    return requiredDeviceExtensions_;
  }

  /// Returns the Vulkan physical device properties.
  const vk::PhysicalDeviceProperties &vkPhysicalDeviceProperties() const {
    return vkPhysicalDeviceProperties_;
  }

  /// Returns the Vulkan physical device queue family properties.
  const std::vector<vk::QueueFamilyProperties> &vkQueueFamilyProperties()
      const {
    return vkQueueFamilyProperties_;
  }

  /// Returns the Vulkan device extension properties.
  const std::vector<vk::ExtensionProperties> &vkDeviceExtensionProperties()
      const {
    return vkDeviceExtensionProperties_;
  }

  /**
   * Returns the surface capabilities for this physical device.
   * Only call this function if supportsPresentation() returns true.
   * @return The surface capabilities if the surface requires swapchain support.
   * @throws NoPresentationSupport If presentation is not supported.
   */
  const vk::SurfaceCapabilitiesKHR &vkSurfaceCapabilities() const {
    if (supportsPresentation()) {
      return vkSurfaceCapabilities_;
    }
    throw error::NoPresentationSupport("No surface capabilities available");
  }

  /**
   * Returns the surface formats for this physical device.
   * Only call this function if supportsPresentation() returns true.
   * @return The surface formats if the surface requires swapchain support.
   * @throws NoPresentationSupport If presentation is not supported.
   */
  const std::vector<vk::SurfaceFormatKHR> &vkSurfaceFormats() const {
    if (supportsPresentation()) {
      return vkSurfaceFormats_;
    }
    throw error::NoPresentationSupport("No surface formats available");
  }

  /**
   * Returns the surface presentation modes for this physical device.
   * Only call this function if supportsPresentation() returns true.
   * @return The surface presentation modes if the surface requires swapchain
   *   support.
   * @throws NoPresentationSupport If presentation is not supported.
   */
  const std::vector<vk::PresentModeKHR> &vkSurfacePresentModes() const {
    if (supportsPresentation()) {
      return vkSurfacePresentModes_;
    }
    throw error::NoPresentationSupport("No present modes available");
  }

 private:
  /**
   * Chooses the compute queue family from the given families.
   * @param surface Surface to use to check for presentation.
   */
  void chooseComputeQueueFamily_(const Surface &surface);

  /**
   * Chooses the transfer queue family from the given families.
   * Must be called after a compute queue has already been chosen.
   */
  void chooseTransferQueueFamily_();

  /**
   * Chooses the presentation queue family from the given families.
   * Must be called after both the compute queue and transfer queue have already
   * been chosen.
   * @param surface Surface to use to check for presentation.
   */
  void choosePresentationQueueFamily_(const Surface &surface);

  /**
   * Returns whether the given queue family supports presentation.
   * @param surface The surface to check.
   * @param queueFamilyIndex The index of the queue family.
   * @return If the queue family supports presentation or not.
   */
  bool queueFamilySupportsPresentation_(uint32_t queueFamilyIndex,
                                        const Surface &surface);

  /**
   * Checks if all extensions are supported.
   * @throws DeviceExtensionsNotSupported When not all extensions are supported.
   */
  void checkForExtensionSupport_();

  /**
   * Checks if swapchain is supported when the surface requires swapchain.
   * @param surface The surface that requires swapchain support.
   */
  void checkForSwapchainSupport_(const Surface &surface);

  /// Saves the queue family indices in a vector.
  void saveQueueFamilyIndices_();

  vk::PhysicalDevice vkPhysicalDevice_;
  std::vector<const char *> requiredDeviceExtensions_;

  uint32_t computeQueueFamilyIndex_;
  uint32_t transferQueueFamilyIndex_;
  std::vector<uint32_t> queueFamilyIndices_;

  vk::PhysicalDeviceProperties vkPhysicalDeviceProperties_;
  std::vector<vk::QueueFamilyProperties> vkQueueFamilyProperties_;
  std::vector<vk::ExtensionProperties> vkDeviceExtensionProperties_;

  bool hasPresentationQueueFamilyIndex_;
  uint32_t presentationQueueFamilyIndex_;
  vk::SurfaceCapabilitiesKHR vkSurfaceCapabilities_;
  std::vector<vk::SurfaceFormatKHR> vkSurfaceFormats_;
  std::vector<vk::PresentModeKHR> vkSurfacePresentModes_;
};

/**
 * Picks a physical device to be used for rendering.
 *
 * This function selects the first physical device that supports everything
 * needed by Herakles and the given surface.
 *
 * @param surface The surface that needs to be supported by the physical device.
 * @return The selected physical device.
 * @throws error::NoSuitablePhysicalDeviceFoundError If no suitable physical
 *   devices were found.
 */
PhysicalDevice pickPhysicalDevice(const Instance &instance,
                                  const Surface &surface);

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_PHYSICAL_DEVICE_HPP
