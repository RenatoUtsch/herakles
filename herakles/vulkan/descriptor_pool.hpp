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

#ifndef HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_POOL_HPP
#define HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_POOL_HPP

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/descriptor_set_layout.hpp"

namespace hk {

/**
 * A descriptor pool of a given descriptor set layout.
 * The descriptor set layout must live while the descriptor pool lives.
 */
class DescriptorPool {
 public:
  /**
   * Constructs the descriptor pool.
   * @param descriptorSetLayout The descriptor set layout that this pool will
   *   model. Used to get the descriptor pool sizes.
   * @param count The number of descriptor sets that will be created from this
   *   descriptor pool.
   */
  DescriptorPool(const DescriptorSetLayout &descriptorSetLayout,
                 uint32_t count);

  /// Returns the vulkan descriptor pool.
  const vk::DescriptorPool &vkDescriptorPool() const {
    return *descriptorPool_;
  }

  /// Returns the device that this descriptor pool is from.
  const Device &device() const { return device_; }

  /// Returns the descriptor set layout that this descriptor pool is based on.
  const DescriptorSetLayout &descriptorSetLayout() const {
    return descriptorSetLayout_;
  }

 private:
  /// Returns the descriptor pool sizes from the layout bindings.
  /// count is the number of descriptor pools that will be created with this.
  std::vector<vk::DescriptorPoolSize> createPoolSizes_(uint32_t count);

  const Device &device_;
  const DescriptorSetLayout &descriptorSetLayout_;
  vk::UniqueDescriptorPool descriptorPool_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_POOL_HPP
