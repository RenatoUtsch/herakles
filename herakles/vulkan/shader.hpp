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

#ifndef HERAKLES_HERAKLES_VULKAN_SHADER_HPP
#define HERAKLES_HERAKLES_VULKAN_SHADER_HPP

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/device.hpp"
#include "herakles/vulkan/utils.hpp"

namespace hk {

/**
 * Defines a shader to be used with Herakles compute shaders.
 */
class Shader {
 public:
  /**
   * Constructs the shader module and pipeline stage from the given source.
   * @param filename The SPIR-V binary filename to be loaded.
   * @param entryPoint The name of the function that is the entry point of the
   *   shader.
   * @param device The device where the shader will be used.
   */
  Shader(const std::string &filename, const std::string &entryPoint,
         const Device &device)
      : Shader(readBinaryFromFile(filename), entryPoint, device) {}

  /**
   * Constructs the shader module and pipeline stage from the given source.
   * @param code The SPIR-V binary code as a vector of chars.
   * @param entryPoint The name of the function that is the entry point of the
   *   shader.
   * @param device The device where the shader will be used.
   */
  Shader(const std::vector<char> &code, const std::string &entryPoint,
         const Device &device);

  /**
   * Returns the shader stage create info struct for the shader module.
   * TODO(renatoutsch): make this more customizable, letting set constants and
   * the entry point here, and create multiple shader stages from the same
   * shader.
   */
  const vk::PipelineShaderStageCreateInfo &pipelineShaderStageCreateInfo()
      const {
    return pipelineShaderStageCreateInfo_;
  }

 private:
  vk::UniqueShaderModule shaderModule_;
  vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo_;
};

}  // namespace hk

#endif  // HERAKLES_HERAKLES_VULKAN_SHADER_HPP
