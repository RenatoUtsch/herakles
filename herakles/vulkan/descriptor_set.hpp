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

#ifndef HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_HPP
#define HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_HPP

#include <any>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/descriptor_pool.hpp"
#include "herakles/vulkan/descriptor_set_layout.hpp"
#include "herakles/vulkan/device.hpp"

namespace hk {

/**
 * Descriptor set, representing the bindings used by the shaders.
 * The descriptor pool must live while this descriptor set lives.
 * TODO(renatoutsch): add support for updating specific descriptors and copying
 *   descriptors.
 */
class DescriptorSet {
 public:
  /**
   * Constructs the descriptor set from the given descriptor pool.
   * @param descriptorPool The descriptor pool to allocate the descriptor set
   *   from.
   *   @param bindings The descriptor infos for the images/buffers that will
   *     be used in the descriptor pool.
   */
  DescriptorSet(const DescriptorPool &descriptorPool,
                const std::vector<std::any> &descriptorInfos);

  /// Returns the vulkan descriptor set.
  const vk::DescriptorSet &vkDescriptorSet() const { return descriptorSet_; }

 private:
  /// Updates the descriptor set with the given descriptor infos.
  void updateDescriptorSet_(const std::vector<std::any> &descriptorInfos);

  /// Creates the descriptor writing for the given descriptor info and layout
  /// binding.
  vk::WriteDescriptorSet createDescriptorWrite_(
      const std::any &descriptorInfo,
      const vk::DescriptorSetLayoutBinding &layoutBinding);

  const Device &device_;
  const DescriptorSetLayout &descriptorSetLayout_;
  const DescriptorPool &descriptorPool_;
  vk::DescriptorSet descriptorSet_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_DESCRIPTOR_SET_HPP
