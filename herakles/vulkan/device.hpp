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

#ifndef HERAKLES_HERAKLES_VULKAN_DEVICE_HPP
#define HERAKLES_HERAKLES_VULKAN_DEVICE_HPP

#include <functional>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/instance.hpp"
#include "herakles/vulkan/physical_device.hpp"

namespace hk {

/**
 * Represents a single Vulkan logical device.
 * The physical device must be kept alive while the logical device exists.
 */
class Device {
 public:
  /**
   * Constructs a logical device from a suitable physical device.
   * @param instance The instance of the physical device.
   * @param physicalDevice The physical device that this logical device will
   *   represent.
   * @param extraValidationLayers Extra validation layers to be enabled for the
   *   device alone, only if validation layers are enabled for the instance.
   */
  Device(const Instance &instance, const PhysicalDevice &physicalDevice,
         const vk::PhysicalDeviceFeatures &requiredFeatures = {},
         const std::vector<const char *> &extraValidationLayers = {});

  /// Returns the physical device of this device.
  const PhysicalDevice &physicalDevice() const { return physicalDevice_; }

  /// Returns the Vulkan device that this class manages.
  const vk::Device &vkDevice() const { return *vkDevice_; }

  /// Returns the Vulkan compute queue.
  const vk::Queue &vkComputeQueue() const { return vkComputeQueue_; }

  /// Returns the Vulkan compute queue family command pool.
  const vk::CommandPool &vkComputeCommandPool() const {
    return *vkComputeCommandPool_;
  }

  /**
   * Creates a command buffer from the compute command pool.
   * @param count The number of command buffers to create.
   * @param level The command buffer level.
   * @return The vector of created command buffers.
   */
  std::vector<vk::CommandBuffer> allocateComputeCommandBuffers(
      uint32_t count,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
    return allocateCommandBuffers_(*vkComputeCommandPool_, count, level);
  }

  /**
   * Creates a single command buffer from the compute command pool.
   * @param level The command buffer level.
   * @return The created command buffer.
   */
  vk::CommandBuffer allocateComputeCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
    return allocateComputeCommandBuffers(1, level)[0];
  }

  /**
   * Creates and submits an one-time submit compute command buffer.
   * @param commands Functor that receives the created command buffer as
   *   parameter and can specify commands to be executed by the buffer. These
   *   commands will be enqueued to the given queue as soon as they've finished
   *   recording and the queue will be emptied. Please note that the begin() and
   *   end() functions of the command buffer are called automatically.
   */
  void submitOneTimeComputeCommands(
      std::function<void(const vk::CommandBuffer &)> commands) const {
    const auto commandBuffer = allocateComputeCommandBuffer();
    submitOneTimeCommands_(commandBuffer, vkComputeQueue_, commands);
  }

  /// Returns the Vulkan transfer queue.
  const vk::Queue &vkTransferQueue() const { return vkTransferQueue_; }

  /// Returns the Vulkan transfer queue family command pool.
  const vk::CommandPool &vkTransferCommandPool() const {
    return *vkTransferCommandPool_;
  }

  /**
   * Creates a command buffer from the transfer command pool.
   * @param count The number of command buffers to create.
   * @param level The command buffer level.
   * @return The vector of created command buffers.
   */
  std::vector<vk::CommandBuffer> allocateTransferCommandBuffers(
      uint32_t count,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
    return allocateCommandBuffers_(*vkTransferCommandPool_, count, level);
  }

  /**
   * Creates a single command buffer from the transfer command pool.
   * @param level The command buffer level.
   * @return The created command buffer.
   */
  vk::CommandBuffer allocateTransferCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
    return allocateTransferCommandBuffers(1, level)[0];
  }

  /**
   * Creates and submits an one-time submit transfer command buffer.
   * @param commands Functor that receives the created command buffer as
   *   parameter and can specify commands to be executed by the buffer. These
   *   commands will be enqueued to the given queue as soon as they've finished
   *   recording and the queue will be emptied. Please note that the begin() and
   *   end() functions of the command buffer are called automatically.
   */
  void submitOneTimeTransferCommands(
      std::function<void(const vk::CommandBuffer &)> commands) const {
    const auto commandBuffer = allocateTransferCommandBuffer();
    submitOneTimeCommands_(commandBuffer, vkTransferQueue_, commands);
  }

  /**
   * Returns the Vulkan present queue.
   * It only exists if supportsPresentation() returns true.
   */
  const vk::Queue &vkPresentationQueue() const {
    if (supportsPresentation()) {
      return vkPresentationQueue_;
    }
    throw error::NoPresentationSupport("No surface formats available");
  }

  /// Returns if the device supports presentation.
  bool supportsPresentation() const { return supportsPresentation_; }

  /**
   * Returns whether the present queue is the compute queue.
   * This only makes sense if supportsPresentation() returns true.
   */
  bool presentQueueIsComputeQueue() const {
    return presentQueueIsComputeQueue_;
  }

  /**
   * Creates and returns a semaphore.
   */
  vk::UniqueSemaphore createSemaphore() {
    return vkDevice_->createSemaphoreUnique(vk::SemaphoreCreateInfo());
  }

 private:
  /// Sets up queues supported by the device.
  void setUpQueues_();

  /// Sets up command pools for each queue family.
  void setUpCommandPools_();

  /// Allocates command buffers.
  std::vector<vk::CommandBuffer> allocateCommandBuffers_(
      const vk::CommandPool &commandPool, uint32_t count,
      vk::CommandBufferLevel level) const;

  /// Submits one time commands. commandBuffer must not have been recorded.
  /// TODO(renatoutsch): improve this to support semaphores and fences.
  void submitOneTimeCommands_(
      const vk::CommandBuffer &commandBuffer, const vk::Queue &queue,
      std::function<void(const vk::CommandBuffer &)> commands) const;

  const PhysicalDevice &physicalDevice_;

  vk::UniqueDevice vkDevice_;
  vk::Queue vkComputeQueue_;
  vk::Queue vkTransferQueue_;
  vk::Queue vkPresentationQueue_;
  vk::UniqueCommandPool vkComputeCommandPool_;
  vk::UniqueCommandPool vkTransferCommandPool_;

  bool supportsPresentation_;
  bool presentQueueIsComputeQueue_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_DEVICE_HPP
