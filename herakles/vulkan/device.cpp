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

#include "herakles/vulkan/device.hpp"

namespace hk {
namespace {

/// Joins the vectors of validation layers into one vector only.
std::vector<const char *> getRequiredValidationLayers_(
    const std::vector<const char *> &instanceValidationLayers,
    const std::vector<const char *> &extraValidationLayers) {
  std::vector<const char *> validationLayers(instanceValidationLayers.begin(),
                                             instanceValidationLayers.end());
  validationLayers.insert(validationLayers.end(), extraValidationLayers.begin(),
                          extraValidationLayers.end());

  return validationLayers;
}

}  // namespace

Device::Device(const Instance &instance, const PhysicalDevice &physicalDevice,
               const vk::PhysicalDeviceFeatures &requiredFeatures,
               const std::vector<const char *> &extraValidationLayers)
    : physicalDevice_(physicalDevice) {
  vk::DeviceCreateInfo createInfo;
  createInfo.setPEnabledFeatures(&requiredFeatures);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  const float queuePriority = 1.0f;
  for (uint32_t queueFamilyIndex : physicalDevice.queueFamilyIndices()) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
        .setQueueCount(1)
        .setPQueuePriorities(&queuePriority);

    queueCreateInfos.push_back(queueCreateInfo);
  }

  createInfo.setQueueCreateInfoCount(queueCreateInfos.size())
      .setPQueueCreateInfos(queueCreateInfos.data());

  const auto &requiredExtensions = physicalDevice.requiredDeviceExtensions();
  createInfo.setEnabledExtensionCount(requiredExtensions.size())
      .setPpEnabledExtensionNames(requiredExtensions.data());

  const auto validationLayers = getRequiredValidationLayers_(
      instance.validationLayers(), extraValidationLayers);
  if (instance.validationLayersEnabled()) {
    createInfo.setEnabledLayerCount(validationLayers.size())
        .setPpEnabledLayerNames(validationLayers.data());
  }

  vkDevice_ = physicalDevice.vkPhysicalDevice().createDeviceUnique(createInfo);
  setUpQueues_();
  setUpCommandPools_();
}

void Device::setUpQueues_() {
  vkComputeQueue_ =
      vkDevice_->getQueue(physicalDevice_.computeQueueFamilyIndex(), 0);
  vkTransferQueue_ =
      vkDevice_->getQueue(physicalDevice_.transferQueueFamilyIndex(), 0);

  supportsPresentation_ = physicalDevice_.supportsPresentation();
  if (supportsPresentation_) {
    vkPresentationQueue_ =
        vkDevice_->getQueue(physicalDevice_.presentationQueueFamilyIndex(), 0);

    presentQueueIsComputeQueue_ =
        physicalDevice_.computeQueueFamilyIndex() ==
        physicalDevice_.presentationQueueFamilyIndex();
  }
}

void Device::setUpCommandPools_() {
  vk::CommandPoolCreateInfo computePoolInfo;
  computePoolInfo.setQueueFamilyIndex(
      physicalDevice_.computeQueueFamilyIndex());

  vkComputeCommandPool_ = vkDevice_->createCommandPoolUnique(computePoolInfo);

  vk::CommandPoolCreateInfo transferPoolInfo;
  transferPoolInfo.setQueueFamilyIndex(
      physicalDevice_.transferQueueFamilyIndex());

  vkTransferCommandPool_ = vkDevice_->createCommandPoolUnique(transferPoolInfo);
}

std::vector<vk::CommandBuffer> Device::allocateCommandBuffers_(
    const vk::CommandPool &commandPool, uint32_t count,
    vk::CommandBufferLevel level) const {
  vk::CommandBufferAllocateInfo allocateInfo;
  allocateInfo.setCommandPool(commandPool)
      .setCommandBufferCount(count)
      .setLevel(level);

  return vkDevice_->allocateCommandBuffers(allocateInfo);
}

void Device::submitOneTimeCommands_(
    const vk::CommandBuffer &commandBuffer, const vk::Queue &queue,
    std::function<void(const vk::CommandBuffer &)> commands) const {
  commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commands(commandBuffer);
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
  queue.submit(1, &submitInfo, nullptr);
}

}  // namespace hk
