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

#include "herakles/vulkan/shader.hpp"

namespace hk {

Shader::Shader(const std::vector<char> &code, const std::string &entryPoint,
               const Device &device) {
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCodeSize(code.size())
      .setPCode(reinterpret_cast<const uint32_t *>(code.data()));

  shaderModule_ = device.vkDevice().createShaderModuleUnique(createInfo);

  pipelineShaderStageCreateInfo_.setStage(vk::ShaderStageFlagBits::eCompute)
      .setModule(*shaderModule_)
      .setPName(entryPoint.data());
}

}  // namespace hk
