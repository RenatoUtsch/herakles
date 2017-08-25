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

#ifndef HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_LAYOUT_HPP
#define HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_LAYOUT_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/device.hpp"

namespace hk {

/**
 * Represents a Vulkan descriptor set layout used in a pipeline.
 * The Device used to create the descriptor set layout must live while the
 * descriptor set layout lives.
 */
class DescriptorSetLayout {
 public:
  /**
   * Creates a descriptor set layout usable in an herakles pipeline.
   * @param device The device that will use the descriptor set.
   * @param bindings The bindings used in the layout. It is only required to set
   *   the binding, descriptorType and descriptorCount parameters (the first 3
   *   parameters of the constructor). The stage will be set automatically, and
   *   the other parameters are optional.
   */
  DescriptorSetLayout(
      const Device &device,
      const std::vector<vk::DescriptorSetLayoutBinding> &bindings);

  /// Returns the Vulkan descriptor set layout.
  const vk::DescriptorSetLayout &vkDescriptorSetLayout() const {
    return *descriptorSetLayout_;
  }

  /// Returns the bindings used in this layout.
  const std::vector<vk::DescriptorSetLayoutBinding> &bindings() const {
    return bindings_;
  }

  /// Returns the device used to create this layout.
  const Device &device() const { return device_; }

 private:
  const Device &device_;
  std::vector<vk::DescriptorSetLayoutBinding> bindings_;
  vk::UniqueDescriptorSetLayout descriptorSetLayout_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_LAYOUT_HPP
