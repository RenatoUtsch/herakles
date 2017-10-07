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

#include "herakles/vulkan/buffer.hpp"

namespace hk {

Buffer::Buffer(const Device &device, const vk::DeviceSize &size,
               const vk::BufferUsageFlags &usage,
               const std::vector<uint32_t> &queueFamilyIndices)
    : device_(device), requestedSize_(size) {
  vk::BufferCreateInfo createInfo;
  createInfo.setSize(size).setUsage(usage);

  if (queueFamilyIndices.empty()) {
    createInfo.setSharingMode(vk::SharingMode::eExclusive);
  } else {
    createInfo.setSharingMode(vk::SharingMode::eConcurrent)
        .setQueueFamilyIndexCount(queueFamilyIndices.size())
        .setPQueueFamilyIndices(queueFamilyIndices.data());
  }

  vkBuffer_ = device.vkDevice().createBufferUnique(createInfo);
  memoryRequirements_ =
      device.vkDevice().getBufferMemoryRequirements(*vkBuffer_);
}

void Buffer::registerDeviceMemory(
    std::shared_ptr<vk::UniqueDeviceMemory> deviceMemory,
    const vk::DeviceSize &offset) {
  device_.vkDevice().bindBufferMemory(*vkBuffer_, **deviceMemory, offset);

  deviceMemory_ = deviceMemory;
  memoryOffset_ = offset;
}

void Buffer::mapMemory(std::function<void(void *)> functor,
                       vk::DeviceSize bufferOffset, vk::DeviceSize size) const {
  const auto deviceMemory = **deviceMemory_;
  const auto &mappingSize = size ? size : memoryRequirements_.size;

  void *data = device_.vkDevice().mapMemory(
      deviceMemory, memoryOffset_ + bufferOffset, mappingSize);

  functor(data);

  device_.vkDevice().unmapMemory(deviceMemory);
}

void Buffer::copyTo(const vk::CommandBuffer &commandBuffer,
                    const Buffer &dstBuffer) const {
  vk::BufferCopy copyRegion;
  copyRegion.setSize(memoryRequirements_.size);

  commandBuffer.copyBuffer(*vkBuffer_, dstBuffer.vkBuffer(), 1, &copyRegion);
}

void Buffer::copyTo(const vk::CommandBuffer &commandBuffer,
                    const Image &dstImage,
                    const vk::ImageLayout &imageLayout) const {
  vk::BufferImageCopy copyRegion;

  copyRegion.setImageSubresource(dstImage.subresource())
      .setImageExtent(dstImage.extent());

  commandBuffer.copyBufferToImage(*vkBuffer_, dstImage.vkImage(), imageLayout,
                                  1, &copyRegion);
}

}  // namespace hk
