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

#include "herakles/vulkan/swapchain.hpp"

#include <algorithm>
#include <limits>

#include <glog/logging.h>

namespace hk {
namespace {

// Selects the ideal surface format from the available ones.
vk::SurfaceFormatKHR chooseSurfaceFormat_(
    const std::vector<vk::SurfaceFormatKHR> &formats) {
  if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
    // We're free to choose the format.
    LOG(INFO) << "Swapchain format: B8G8R8A8Unorm";
    LOG(INFO) << "Swapchain color space: SrgbNonlinear";
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for (const auto &format : formats) {
    if (format.format == vk::Format::eB8G8R8A8Unorm &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      LOG(INFO) << "Swapchain format: B8G8R8A8Unorm";
      LOG(INFO) << "Swapchain color space: SrgbNonlinear";
      return format;
    }
  }

  LOG(WARNING) << "Ideal swapchain format not found. Selecting first...";
  CHECK(!formats.empty()) << "At least one format should be supported";
  LOG(INFO) << "Swapchain format: " << (int)formats[0].format;
  LOG(INFO) << "Swapchain color space: " << (int)formats[0].colorSpace;
  return formats[0];
}

// Selects the ideal present mode from the available ones.
vk::PresentModeKHR choosePresentMode_(
    const std::vector<vk::PresentModeKHR> &presentModes) {
  for (const auto &mode : presentModes) {
    if (mode == vk::PresentModeKHR::eMailbox) {
      LOG(INFO) << "Swapchain present mode: Mailbox";
      return mode;
    }
  }

  // Fifo is required to be supported.
  LOG(INFO) << "Swapchain present mode: Fifo";
  return vk::PresentModeKHR::eFifo;
}

// Selects the ideal extent from the swapchain capabilities and specified
// width and height.
vk::Extent2D chooseExtent_(const vk::SurfaceCapabilitiesKHR &capabilities,
                           uint32_t width, uint32_t height) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    // We can't change the extent, keep the one returned by the swapchain.
    LOG(INFO) << "Swapchain extent can't be customized. Keeping default...";
    return capabilities.currentExtent;
  }

  // Try to choose the closest size to the given width and height.
  vk::Extent2D extent = {width, height};
  extent.width =
      std::max(capabilities.minImageExtent.width,
               std::min(capabilities.maxImageExtent.width, extent.width));
  extent.height =
      std::max(capabilities.minImageExtent.height,
               std::min(capabilities.maxImageExtent.height, extent.height));

  LOG(INFO) << "Swapchain extent: width " << extent.width << "px | height "
            << extent.height << "px";
  return extent;
}

// Selects the ideal image count for the swapchain.
// Tries to get one more than the minimum image count to implement double/triple
// buffering properly.
uint32_t chooseImageCount_(const vk::SurfaceCapabilitiesKHR &capabilities) {
  uint32_t imageCount = capabilities.minImageCount + 2;
  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  LOG(INFO) << "Swapchain image count: " << imageCount;
  return imageCount;
}

}  // namespace

Swapchain::Swapchain(const Surface &surface, const Device &device)
    : surface_(surface),
      device_(device),
      physicalDevice_(device.physicalDevice()),
      capabilities_(physicalDevice_.vkSurfaceCapabilities()),
      surfaceFormat_(chooseSurfaceFormat_(physicalDevice_.vkSurfaceFormats())),
      presentMode_(choosePresentMode_(physicalDevice_.vkSurfacePresentModes())),
      extent_(chooseExtent_(capabilities_, surface_.requestedWidth(),
                            surface_.requestedHeight())) {
  const uint32_t imageCount = chooseImageCount_(capabilities_);

  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.setSurface(surface.vkSurface())
      .setMinImageCount(imageCount)
      .setImageFormat(surfaceFormat_.format)
      .setImageColorSpace(surfaceFormat_.colorSpace)
      .setImageExtent(extent_)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eStorage |
                     vk::ImageUsageFlagBits::eTransferDst)
      .setImageSharingMode(vk::SharingMode::eExclusive)
      .setPreTransform(capabilities_.currentTransform)
      .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
      .setPresentMode(presentMode_)
      .setClipped(true);
  // TODO(renatoutsch): add old swapchain here.

  vkSwapchain_ = device.vkDevice().createSwapchainKHRUnique(createInfo);
  const auto vkImages = device.vkDevice().getSwapchainImagesKHR(*vkSwapchain_);

  for (const auto &vkImage : vkImages) {
    auto image = Image(device, extent_.width, extent_.height, vkImage,
                       surfaceFormat_.format);
    imageViews_.push_back(image.createImageView());
    images_.push_back(std::move(image));
  }

  // Set up the prebaked vkPresentInfo.
  vkPresentInfo_.setSwapchainCount(1).setPSwapchains(&*vkSwapchain_);

  LOG(INFO) << "Created swapchain";
  LOG(INFO) << "Swapchain width: " << extent_.width;
  LOG(INFO) << "Swapchain height: " << extent_.height;
}

}  // namespace hk
