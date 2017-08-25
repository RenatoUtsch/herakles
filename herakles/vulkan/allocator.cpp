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

#include "herakles/vulkan/allocator.hpp"

#include <utility>

#include <glog/logging.h>

#include "herakles/vulkan/physical_device.hpp"

namespace hk {
namespace {

/**
 * Returns the memory size and type filter from the given array proxy.
 * The objects in the proxy must have the memoryRequirements() function, that
 * returns correct vk::MemoryRequirements for the object to be allocated.
 * @param objects Objects to get the memory requirements from.
 * @returns A pair of (allocationSize, typeFilter), where allocationSize is the
 *   size in bytes the allocation should have and typeFilter the filter for all
 *   memory requirements of all objects.
 */
template <class T>
std::pair<vk::DeviceSize, uint32_t> fullMemoryRequirements_(
    const std::vector<std::reference_wrapper<T>> &objects) {
  vk::DeviceSize memorySize = 0;
  uint32_t typeFilter = 0;
  for (const auto &object : objects) {
    const vk::MemoryRequirements &memoryRequirements =
        object.get().memoryRequirements();
    memorySize += memoryRequirements.size;
    typeFilter |= memoryRequirements.memoryTypeBits;
  }

  return {memorySize, typeFilter};
}

/// Finds the ideal memory type for the given filter and properties.
uint32_t findMemoryType_(const PhysicalDevice &physicalDevice,
                         uint32_t typeFilter,
                         const vk::MemoryPropertyFlags &properties) {
  const auto memoryProperties =
      physicalDevice.vkPhysicalDevice().getMemoryProperties();

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    const auto &memoryType = memoryProperties.memoryTypes[i];
    if ((typeFilter & (1 << i)) &&
        (memoryType.propertyFlags & properties) == properties) {
      return i;
    }
  }

  LOG(FATAL) << "Could not find a suitable memory type with type filter "
             << typeFilter << " and memory property flags "
             << (unsigned)properties;
}

/// Allocates and returns device memory.
SharedDeviceMemory allocateDeviceMemory_(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const vk::DeviceSize &allocationSize, uint32_t typeFilter) {
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(allocationSize)
      .setMemoryTypeIndex(
          findMemoryType_(device.physicalDevice(), typeFilter, properties));

  return std::make_shared<vk::UniqueDeviceMemory>(
      device.vkDevice().allocateMemoryUnique(allocInfo));
}

/**
 * Allocates memory and registers it to a device for the given objects.
 * The objects must have the memoryRequirements() and registerDeviceMemory()
 * function.
 */
template <class T>
SharedDeviceMemory allocateAndRegisterMemory_(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const std::vector<std::reference_wrapper<T>> &objects) {
  auto[allocationSize, typeFilter] = fullMemoryRequirements_(objects);
  auto deviceMemory =
      allocateDeviceMemory_(device, properties, allocationSize, typeFilter);

  vk::DeviceSize offset = 0;
  for (auto &object : objects) {
    object.get().registerDeviceMemory(deviceMemory, offset);
    offset += object.get().memoryRequirements().size;
  }

  return deviceMemory;
}

}  // namespace

SharedDeviceMemory allocateMemory(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const std::vector<std::reference_wrapper<Buffer>> &buffers) {
  return allocateAndRegisterMemory_(device, properties, buffers);
}

SharedDeviceMemory allocateMemory(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const std::vector<std::reference_wrapper<Image>> &images) {
  return allocateAndRegisterMemory_(device, properties, images);
}

}  // namespace hk
