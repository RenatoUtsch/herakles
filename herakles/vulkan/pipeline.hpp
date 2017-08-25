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

#ifndef HERAKLES_HERAKLES_VULKAN_PIPELINE_HPP
#define HERAKLES_HERAKLES_VULKAN_PIPELINE_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/descriptor_set_layout.hpp"
#include "herakles/vulkan/device.hpp"
#include "herakles/vulkan/shader.hpp"

namespace hk {

/**
 * Represents an Herakles compute pipeline.
 */
class Pipeline {
 public:
  /**
   * Create the compute pipeline.
   * @param device The device where the pipeline will run on.
   * @param shader The shader executed in the pipeline.
   * @param descriptorSetLayout The descriptor set layout used in the pipeline.
   *   TODO(renatoutsch): extend this to support multiple descriptor set
   *     layouts.
   *   TODO(renatoutsch): extend this to support multiple push constants.
   */
  Pipeline(const Device &device, const Shader &shader,
           const DescriptorSetLayout &descriptorSetLayout,
           const vk::PushConstantRange &pushConstantRange = {});

  /// Returns the vulkan pipeline layout.
  const vk::PipelineLayout &vkPipelineLayout() const {
    return *pipelineLayout_;
  }

  /// Returns the vulkan pipeline.
  const vk::Pipeline &vkPipeline() const { return *pipeline_; }

 private:
  vk::UniquePipelineLayout pipelineLayout_;
  vk::UniquePipeline pipeline_;
};

}  // namespace hk

#endif  // HERAKLES_HERAKLES_VULKAN_PIPELINE_HPP
