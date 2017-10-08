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

#include "herakles/vulkan/descriptor_pool.hpp"

#include <glog/logging.h>

namespace hk {

DescriptorPool::DescriptorPool(const DescriptorSetLayout &descriptorSetLayout,
                               uint32_t count)
    : device_(descriptorSetLayout.device()),
      descriptorSetLayout_(descriptorSetLayout) {
  const auto &poolSizes = createPoolSizes_(count);

  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setPoolSizeCount(poolSizes.size())
      .setPPoolSizes(poolSizes.data())
      .setMaxSets(count);

  descriptorPool_ = device_.vkDevice().createDescriptorPoolUnique(createInfo);
  LOG(INFO) << "Created descriptor pool";
}

std::vector<vk::DescriptorPoolSize> DescriptorPool::createPoolSizes_(
    uint32_t count) {
  std::vector<vk::DescriptorPoolSize> poolSizes;
  for (const auto &binding : descriptorSetLayout_.bindings()) {
    vk::DescriptorPoolSize poolSize;
    poolSize.setType(binding.descriptorType)
        .setDescriptorCount(binding.descriptorCount * count);

    poolSizes.push_back(poolSize);
  }

  return poolSizes;
}

}  // namespace hk
