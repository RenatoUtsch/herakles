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

#ifndef HERAKLES_HERAKLES_VULKAN_ALLOCATOR_HPP
#define HERAKLES_HERAKLES_VULKAN_ALLOCATOR_HPP

#include <functional>
#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/buffer.hpp"
#include "herakles/vulkan/image.hpp"

namespace hk {

/// Type used to device memory between objects.
using SharedDeviceMemory = std::shared_ptr<vk::UniqueDeviceMemory>;

/**
 * Allocates and binds the exact amount of memory needed by the buffers.
 * This will automatically call registerDeviceMemory() with the correct offsets.
 * The buffers must NOT have already been bound to memory.
 * @param device The device that will provide the memory. All buffers must have
 *   been created from this same device.
 * @param properties The properties this device memory should have.
 * @param buffers The buffers to be bound to this new allocation.
 */
SharedDeviceMemory allocateMemory(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const std::vector<std::reference_wrapper<Buffer>> &buffers);

/**
 * Allocates and binds the exact amount of memory needed by the images.
 * This will automatically call registerDeviceMemory() with the correct offsets.
 * The images must NOT have already been bound to memory.
 * Take care, because images like swapchain images are automatically bound to
 * memory and should not be used here.
 * @param device The device that will provide the memory. All images must have
 *   been created from this same device.
 * @param properties The properties this device memory should have.
 * @param images The images to be bound to this new allocation.
 */
SharedDeviceMemory allocateMemory(
    const Device &device, const vk::MemoryPropertyFlags &properties,
    const std::vector<std::reference_wrapper<Image>> &images);

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_ALLOCATOR_HPP
