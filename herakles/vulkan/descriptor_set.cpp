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

#include "herakles/vulkan/descriptor_set.hpp"

#include <typeinfo>

#include <glog/logging.h>

namespace hk {

DescriptorSet::DescriptorSet(const DescriptorPool &descriptorPool,
                             const std::vector<std::any> &descriptorInfos)
    : device_(descriptorPool.device()),
      descriptorSetLayout_(descriptorPool.descriptorSetLayout()),
      descriptorPool_(descriptorPool) {
  vk::DescriptorSetAllocateInfo allocInfo;
  allocInfo.setDescriptorPool(descriptorPool_.vkDescriptorPool())
      .setDescriptorSetCount(1)
      .setPSetLayouts(&descriptorSetLayout_.vkDescriptorSetLayout());

  auto descriptorSets = device_.vkDevice().allocateDescriptorSets(allocInfo);
  descriptorSet_ = std::move(descriptorSets[0]);

  updateDescriptorSet_(descriptorInfos);
}

void DescriptorSet::updateDescriptorSet_(
    const std::vector<std::any> &descriptorInfos) {
  const auto &layoutBindings = descriptorSetLayout_.bindings();
  CHECK(descriptorInfos.size() == layoutBindings.size())
      << "Descriptor infos and layout bindings are not the same size";

  std::vector<vk::WriteDescriptorSet> descriptorWrites;
  for (uint32_t i = 0; i < descriptorInfos.size(); ++i) {
    descriptorWrites.push_back(
        createDescriptorWrite_(descriptorInfos[i], layoutBindings[i]));
  }

  device_.vkDevice().updateDescriptorSets(descriptorWrites.size(),
                                          descriptorWrites.data(), 0, nullptr);
}

vk::WriteDescriptorSet DescriptorSet::createDescriptorWrite_(
    const std::any &descriptorInfo,
    const vk::DescriptorSetLayoutBinding &layoutBinding) {
  vk::WriteDescriptorSet write;
  write.setDstSet(descriptorSet_)
      .setDstBinding(layoutBinding.binding)
      .setDstArrayElement(0)
      .setDescriptorType(layoutBinding.descriptorType)
      .setDescriptorCount(1);

  switch (layoutBinding.descriptorType) {
    case vk::DescriptorType::eSampler:
    case vk::DescriptorType::eCombinedImageSampler:
    case vk::DescriptorType::eSampledImage:
    case vk::DescriptorType::eStorageImage:
    case vk::DescriptorType::eInputAttachment:
      write.setPImageInfo(
          std::any_cast<vk::DescriptorImageInfo>(&descriptorInfo));
      break;

    case vk::DescriptorType::eUniformBuffer:
    case vk::DescriptorType::eStorageBuffer:
    case vk::DescriptorType::eUniformBufferDynamic:
    case vk::DescriptorType::eStorageBufferDynamic:
      write.setPBufferInfo(
          std::any_cast<vk::DescriptorBufferInfo>(&descriptorInfo));
      break;

    case vk::DescriptorType::eUniformTexelBuffer:
    case vk::DescriptorType::eStorageTexelBuffer:
      write.setPTexelBufferView(std::any_cast<vk::BufferView>(&descriptorInfo));
      break;
  }

  return write;
}

}  // namespace hk
