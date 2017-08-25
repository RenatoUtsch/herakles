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

#ifndef HERAKLES_HERAKLES_VULKAN_BUFFER_HPP
#define HERAKLES_HERAKLES_VULKAN_BUFFER_HPP

#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/device.hpp"
#include "herakles/vulkan/image.hpp"

namespace hk {

/**
 * Wrapper for a Vulkan buffer.
 */
class Buffer {
 public:
  /**
   * Creates a buffer.
   * This buffer is not automatically binded to device memory, and you should
   * specify it when creating a hk::DeviceMemory class before it can be used.
   * @param device Device that the buffer will be tied to.
   * @param size Size in bytes of the buffer.
   * @param usage How the buffer will be used.
   * @param queueFamilyIndices Index of queue families that can access the
   *   buffer concurrently. If no queue families are specified, the buffer will
   *   be created with exclusive sharing mode.
   */
  Buffer(const Device &device, const vk::DeviceSize &size,
         const vk::BufferUsageFlags &usage,
         const std::vector<uint32_t> &queueFamilyIndices = {});

  /**
   * Registers device memory for this buffer.
   * Look at allocator.hpp for an easy way of using this.
   * This buffer will be tied to this device memory until it is deleted. You
   * can't per the spec rebind a buffer to another device memory.
   * @param deviceMemory Pointer do device memory that is shared by all
   *   allocations and is only destroyed when the last allocation using it is
   *   destroyed.
   * @param offset The offset into the memory.
   */
  void registerDeviceMemory(
      std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory,
      const vk::DeviceSize &offset);

  /**
   * Binds and unbinds device memory to host.
   * The buffer MUST be on host visible and coherent memory for this to work.
   * @param functor Functor that receives a pointer to void as the mapped data.
   * @param bufferOffset Offset INTO THE BUFFER's memory (not the device's
   *   memory). If 0, will start from the beginning of the buffer.
   * @param size Number of bytes to map. If 0, will map the entire buffer.
   */
  void mapMemory(std::function<void(void *)> functor,
                 vk::DeviceSize bufferOffset = 0,
                 vk::DeviceSize size = 0) const;

  /**
   * Copies this entire buffer to destBuffer.
   * This only adds the command to the command buffer, and does not schedule
   * anything for execution.
   */
  void copyTo(const vk::CommandBuffer &commandBuffer, const Buffer &dstBuffer);

  /**
   * Copies this entire buffer to destImage.
   * This only adds the command to the command buffer, and does not schedule
   * anything for execution.
   */
  void copyTo(const vk::CommandBuffer &commandBuffer, const Image &dstImage,
              const vk::ImageLayout &imageLayout =
                  vk::ImageLayout::eTransferDstOptimal);

  /// Returns the vulkan buffer.
  const vk::Buffer &vkBuffer() const { return *vkBuffer_; }

  /// Returns the memory requirements of the buffer.
  const vk::MemoryRequirements &memoryRequirements() const {
    return memoryRequirements_;
  }

  /// Returns the memory requirements size.
  const vk::DeviceSize &size() const { return memoryRequirements_.size; }

  /**
   * Returns the device memory bound to the allocation.
   * This will NOT return the device memory if buffer was not bound to memory
   * yet.
   */
  const vk::DeviceMemory &deviceMemory() const { return **deviceMemory_; }

  /// Returns the image's offset into the device memory.
  const vk::DeviceSize &memoryOffset() const { return memoryOffset_; }

 private:
  const Device &device_;
  std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory_;
  vk::DeviceSize memoryOffset_;

  vk::UniqueBuffer vkBuffer_;
  vk::MemoryRequirements memoryRequirements_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_BUFFER_HPP
