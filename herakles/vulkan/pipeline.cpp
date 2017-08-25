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

#include "herakles/vulkan/pipeline.hpp"

#include <glog/logging.h>

namespace hk {

Pipeline::Pipeline(const Device &device, const Shader &shader,
                   const DescriptorSetLayout &descriptorSetLayout,
                   const vk::PushConstantRange &pushConstantRange) {
  vk::PipelineLayoutCreateInfo layoutCreateInfo;
  layoutCreateInfo.setSetLayoutCount(1).setPSetLayouts(
      &descriptorSetLayout.vkDescriptorSetLayout());

  if (pushConstantRange.size) {
    layoutCreateInfo.setPushConstantRangeCount(1).setPPushConstantRanges(
        &pushConstantRange);
  }

  pipelineLayout_ =
      device.vkDevice().createPipelineLayoutUnique(layoutCreateInfo);

  vk::ComputePipelineCreateInfo pipelineCreateInfo;
  pipelineCreateInfo.setStage(shader.pipelineShaderStageCreateInfo())
      .setLayout(*pipelineLayout_);

  pipeline_ = device.vkDevice().createComputePipelineUnique(nullptr,
                                                            pipelineCreateInfo);
  LOG(INFO) << "Created compute pipeline";
}

}  // namespace hk
