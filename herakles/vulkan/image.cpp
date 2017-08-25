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

#include "herakles/vulkan/image.hpp"

namespace hk {
namespace {

// Returns a simple vk::ImageSubresourceLayers without mipmap or layers.
vk::ImageSubresourceLayers createSubresource_() {
  vk::ImageSubresourceLayers subresource;
  subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setMipLevel(0)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

  return subresource;
}

}  // namespace

Image::Image(const Device &device, uint32_t width, uint32_t height,
             const vk::ImageUsageFlags &usage, vk::Format format,
             const std::vector<uint32_t> &queueFamilyIndices)
    : device_(device),
      shouldDestroyImage_(true),
      deleter_(device.vkDevice()),
      format_(format),
      subresource_(createSubresource_()),
      extent_(width, height, 1) {
  vk::ImageCreateInfo createInfo;
  createInfo.setImageType(vk::ImageType::e2D)
      .setFormat(format)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setUsage(usage)
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(vk::SampleCountFlagBits::e1);
  createInfo.extent.setWidth(width).setHeight(height).setDepth(1);

  if (queueFamilyIndices.empty()) {
    createInfo.setSharingMode(vk::SharingMode::eExclusive);
  } else {
    createInfo.setSharingMode(vk::SharingMode::eConcurrent)
        .setQueueFamilyIndexCount(queueFamilyIndices.size())
        .setPQueueFamilyIndices(queueFamilyIndices.data());
  }

  vkImage_ = device.vkDevice().createImage(createInfo);
  memoryRequirements_ = device.vkDevice().getImageMemoryRequirements(vkImage_);
}

Image::Image(const Device &device, uint32_t width, uint32_t height,
             const vk::Image &vkImage, vk::Format format)
    : device_(device),
      shouldDestroyImage_(false),
      vkImage_(vkImage),
      format_(format),
      subresource_(createSubresource_()),
      extent_(width, height, 1) {}

Image::Image(Image &&other) noexcept
    : device_(other.device_),
      shouldDestroyImage_(other.shouldDestroyImage_),
      deleter_(other.deleter_),
      vkImage_(other.vkImage_),
      format_(other.format_),
      subresource_(other.subresource_),
      extent_(other.extent_),
      memoryRequirements_(other.memoryRequirements_) {
  other.vkImage_ = nullptr;
}

Image::~Image() {
  if (shouldDestroyImage_) deleter_(vkImage_);
}

void Image::registerDeviceMemory(
    std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory,
    const vk::DeviceSize &offset) {
  device_.vkDevice().bindImageMemory(vkImage_, **deviceMemory, offset);

  deviceMemory_ = deviceMemory;
  memoryOffset_ = offset;
}

vk::UniqueImageView Image::createImageView() const {
  vk::ImageViewCreateInfo createInfo;
  createInfo.setImage(vkImage_)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format_);
  createInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

  return device_.vkDevice().createImageViewUnique(createInfo);
}

void Image::layoutTransitionBarrier(
    const vk::CommandBuffer &commandBuffer, vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout, const vk::AccessFlags &srcAccessMask,
    const vk::AccessFlags &dstAccessMask,
    const vk::PipelineStageFlags &srcStageMask,
    const vk::PipelineStageFlags &dstStageMask,
    const vk::ImageAspectFlags &aspectMask) const {
  vk::ImageMemoryBarrier barrier;
  barrier.setOldLayout(oldLayout)
      .setNewLayout(newLayout)
      .setSrcAccessMask(srcAccessMask)
      .setDstAccessMask(dstAccessMask)
      .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
      .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
      .setImage(vkImage_);
  barrier.subresourceRange.setAspectMask(aspectMask)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

  commandBuffer.pipelineBarrier(srcStageMask, dstStageMask,
                                vk::DependencyFlags(), 0, nullptr, 0, nullptr,
                                1, &barrier);
}

void Image::copyTo(const vk::CommandBuffer &commandBuffer, const Image &image,
                   const vk::ImageLayout &srcLayout,
                   const vk::ImageLayout &dstLayout) {
  vk::ImageCopy region;
  region.setSrcSubresource(subresource_)
      .setDstSubresource(image.subresource_)
      .setExtent(extent_);

  commandBuffer.copyImage(vkImage_, srcLayout, image.vkImage(), dstLayout, 1,
                          &region);
}

}  // namespace hk
