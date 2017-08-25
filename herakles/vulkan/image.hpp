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

#ifndef HERAKLES_HERAKLES_VULKAN_IMAGE_HPP
#define HERAKLES_HERAKLES_VULKAN_IMAGE_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/device.hpp"

namespace hk {

/**
 * Wrapper for a Vulkan image.
 */
class Image {
 public:
  /**
   * Creates a simple image with optimal tiling.
   * This image is not automatically binded to device memory, and you should
   * specify it when creating a hk::DeviceMemory class before it can be used.
   * This image will not use mipmap levels or array layers and will use optimal
   * tiling for fast access. Use a staging buffer to transfer data to it, if
   * needed.
   * @param device Device that the image will be tied to.
   * @param width The width of the image.
   * @param height The height of the image.
   * @param usage The flags that describe how the image will be used.
   * @param format The format and type of elements contained in the image.
   * @param queueFamilyIndices Index of queue families that can access the image
   *   concurrently. If no queue families are specified, the image will be
   *  created with exclusive sharing mode.
   */
  Image(const Device &device, uint32_t width, uint32_t height,
        const vk::ImageUsageFlags &usage,
        vk::Format format = vk::Format::eB8G8R8A8Unorm,
        const std::vector<uint32_t> &queueFamilyIndices = {});

  /**
   * Constructs an image from the given Vulkan image.
   * With this constructor, the vkImage's allocation will NOT be managed by this
   * class, and thus functions that depend on that will NOT return meaningful
   * values.
   * @param device Device used to construct the image.
   * @param width Width of the given image.
   * @param height Height of the given image.
   * @param vkImage An already created Vulkan image that is not managed by this
   *   class - it's the user's responsibility to deallocate it after this class
   *   is destroyed.
   * @param format Format of the image.
   */
  Image(const Device &device, uint32_t width, uint32_t height,
        const vk::Image &vkImage, vk::Format format);

  Image(Image &&other) noexcept;

  ~Image();

  Image(const Image &other) = delete;
  Image &operator=(const Image &other) = delete;
  Image &operator=(Image &&other) = delete;

  /**
   * Registers device memory for this image.
   * Look at allocator.hpp for an easy way of using this.
   * This image will be tied to this device memory until it is deleted. You
   * can't per the spec rebind an image to another device memory.
   * Remember that some images are not managed by the user, like swapchain
   * images, and thus are already bound to device memory and do not need this
   * call.
   * @param deviceMemory Pointer do device memory that is shared by all
   *   allocations and is only destroyed when the last allocation using it is
   *   destroyed.
   * @param offset The offset into the memory.
   */
  void registerDeviceMemory(
      std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory,
      const vk::DeviceSize &offset);

  /**
   * Creates a vulkan image view of the image.
   * Only call this function if the Image object is already backed by device
   * memory (with the DeviceMemory class).
   * Also be sure to clean this class up before the device memory or the image
   * are cleaned up.
   */
  vk::UniqueImageView createImageView() const;

  /**
   * Sets up a barrier that transitions the image layout to the given layout.
   * @param commandBuffer The command buffer to record the transition commands.
   * @param oldLayout The old layout of the image. If you don't know the old
   *   layout of the image or don't care about the old contents, use eUndefined.
   *   It will provide the best performance, but you may lose the contents of
   *   the image.
   * @param newLayout The new layout of the image.
   * @param srcAccessMask How the image was being accessed before this
   *   transition. If it was not, use vk::AccessFlags().
   * @param dstAccessMask How the image is going to be used after this
   *   transition.
   * @param srcStageMask Stage that produces the data used after the transition.
   * @param dstStageMask Stage that consumes the transitioned data.
   * @param aspectMask The aspects that will be affected by this transition. By
   *   default, the color components.
   */
  void layoutTransitionBarrier(const vk::CommandBuffer &commandBuffer,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               const vk::AccessFlags &srcAccessMask,
                               const vk::AccessFlags &dstAccessMask,
                               const vk::PipelineStageFlags &srcStageMask =
                                   vk::PipelineStageFlagBits::eBottomOfPipe,
                               const vk::PipelineStageFlags &dstStageMask =
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                               const vk::ImageAspectFlags &aspectMask =
                                   vk::ImageAspectFlagBits::eColor) const;

  /**
   * Copies this entire image to destImage.
   * This only adds the command to the command buffer. It is the responsibility
   * of the caller to make the proper layout transitions.
   * @param commandBuffer Command buffer to record the copy operation.
   * @param destImage Destination of the copy operation.
   * @param srcLayout Layout the source image must be in. Either eGeneral or
   *   eTransferSrcOptimal.
   * @param dstLayout Layout the destination image must be in. Either eGeneral
   *   or eTransferDstOptimal.
   */
  void copyTo(
      const vk::CommandBuffer &commandBuffer, const Image &destImage,
      const vk::ImageLayout &srcLayout = vk::ImageLayout::eTransferSrcOptimal,
      const vk::ImageLayout &dstLayout = vk::ImageLayout::eTransferDstOptimal);

  /// Returns the vulkan image.
  const vk::Image &vkImage() const { return vkImage_; }

  /// Returns the subresource layers of the image.
  const vk::ImageSubresourceLayers &subresource() const { return subresource_; }

  /// Returns the extent of the image.
  const vk::Extent3D &extent() const { return extent_; }

  /**
   * Returns the memory requirements of the image.
   * This will NOT return a meaningful value unless the allocation is managed
   * by this class.
   */
  const vk::MemoryRequirements &memoryRequirements() const {
    return memoryRequirements_;
  }

  /**
   * Returns the size memory requirements size.
   * This will NOT return a meaningful value unless the allocation is managed
   * by this class.
   */
  const vk::DeviceSize &size() const { return memoryRequirements_.size; }

  /**
   * Returns the device memory bound to the allocation.
   * This will NOT return the device memory if the memory isn't managed by this
   * class or if the image was not bound to memory yet.
   */
  const vk::DeviceMemory &deviceMemory() const { return **deviceMemory_; }

  /// Returns the image's offset into the device memory.
  const vk::DeviceSize &memoryOffset() const { return memoryOffset_; }

 private:
  const Device &device_;
  std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory_;
  vk::DeviceSize memoryOffset_;

  const bool shouldDestroyImage_;
  vk::ImageDeleter deleter_;
  vk::Image vkImage_;

  vk::Format format_;
  vk::ImageSubresourceLayers subresource_;
  vk::Extent3D extent_;
  vk::MemoryRequirements memoryRequirements_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_IMAGE_HPP
