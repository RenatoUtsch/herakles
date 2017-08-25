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

#ifndef HERAKLES_HERAKLES_VULKAN_SWAPCHAIN_HPP
#define HERAKLES_HERAKLES_VULKAN_SWAPCHAIN_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "herakles/vulkan/device.hpp"
#include "herakles/vulkan/image.hpp"
#include "herakles/vulkan/surface.hpp"

namespace hk {

/**
 * A Herakles swapchain that represents a Vulkan swapchainKHR object.
 */
class Swapchain {
 public:
  /**
   * Constructs a swapchain from the given surface to the given device.
   * @param surface The surface where the swapchain will be constructed from.
   * @param device The device that will use the swapchain.
   */
  Swapchain(const Surface &surface, const Device &device);

  /**
   * Acquires the next image to present.
   * Returns it's index in the list of available swapchain image views.
   * @param timeout How long the function should wait before returning. If set
   *   to std::numeric_limits<uint64_t>::max(), disables the timeout. If set to
   *   0, the call will not block and either succeed or return eNotReady. For
   *   other values of timeout, the call will either succeed or return after the
   *   specified time with eTimeout.
   * @param semaphore The semaphore to be signaled when the image is acquired.
   *   If the semaphore is used (by initializing it with {}), you must specify a
   *   fence.
   * @param fence The fence to be signaled when the image is acquired. If the
   *   fence is not initialized, you must specify a semaphore.
   * @return The index of the acquired image in return.value if return.result is
   *   not eNotReady or eTimeout (depending on how you set timeout). Please note
   *   that if result is eSuboptimalKHR, the swapchain should to be recreated
   *   (but still works).
   */
  vk::ResultValue<uint32_t> acquireNextImage(
      uint64_t timeout, const vk::Semaphore &semaphore,
      const vk::Fence &fence = {}) const {
    return device_.vkDevice().acquireNextImageKHR(*vkSwapchain_, timeout,
                                                  semaphore, fence);
  }

  /**
   * Presents the image with the given index to the swapchain.
   * @param index The index of the image to present to the swapchain. Must have
   *   previously been acquired with acquireNextImage() and not presented
   *   afterwards.
   * @param numSemaphores Number of specified semaphores.
   * @param semaphores Semaphores to signal when presenting is finished.
   * @return Either eSuccess, meaning that the image was presented successfully
   *   or eSuboptimalKHR, meaning that the call was successful but the swapchain
   *   should be recreated (even if it still works right now).
   */
  vk::Result presentImage(uint32_t index, uint32_t numSemaphores,
                          const vk::Semaphore *semaphores) {
    vkPresentInfo_.setWaitSemaphoreCount(numSemaphores)
        .setPWaitSemaphores(semaphores)
        .setPImageIndices(&index);

    return device_.vkPresentationQueue().presentKHR(vkPresentInfo_);
  }

  /**
   * Returns the number of available images.
   */
  uint32_t numImages() const { return images_.size(); }

  /**
   * Returns the image with the given index.
   * You should only modify images that were previously acquired with
   * acquireNextImage() and not yet presented with presentImage(). There's no
   * harm in referencing the images anywhere without using them, though.
   * @param index The index of the image to return.
   * @return The swapchain vk::Image with the given index.
   */
  const Image &image(uint32_t index) const { return images_[index]; }

  /**
   * Returns the image view with the given index.
   * Remember that you should only modify images that were previously acquired
   * with acquireNextImage() and not yet presented with presentImage(). There's
   * no harm in referencing the image views anywhere without using them, though.
   * @param index The index of the image view to return.
   * @return The swapchain vk::ImageView with the given index.
   */
  const vk::ImageView &imageView(uint32_t index) const {
    return *imageViews_[index];
  }

  /**
   * Returns the real width of the swapchain images.
   * Equivalent to extent().width.
   */
  uint32_t width() const { return extent_.width; }

  /**
   * Returns the real height of the swapchain images.
   * Equivalent to extent().height.
   */
  uint32_t height() const { return extent_.height; }

  /// Returns the extent of the swapchain images.
  const vk::Extent2D &extent() const { return extent_; }

  /// Returns the surface format of the swapchain.
  const vk::SurfaceFormatKHR &surfaceFormat() const { return surfaceFormat_; }

  /// Returns the present mode of the swapchain.
  const vk::PresentModeKHR &presentMode() const { return presentMode_; }

  /// Returns the Vulkan swapchain instance.
  const vk::SwapchainKHR &vkSwapchain() { return *vkSwapchain_; }

  /// Returns the surface of this swapchain.
  const Surface &surface() const { return surface_; }

  /// Returns the device using this swapchain.
  const Device &device() const { return device_; }

 private:
  const Surface &surface_;
  const Device &device_;
  const PhysicalDevice &physicalDevice_;
  const vk::SurfaceCapabilitiesKHR &capabilities_;
  vk::SurfaceFormatKHR surfaceFormat_;
  vk::PresentModeKHR presentMode_;
  vk::Extent2D extent_;
  vk::UniqueSwapchainKHR vkSwapchain_;
  std::vector<Image> images_;
  std::vector<vk::UniqueImageView> imageViews_;
  vk::PresentInfoKHR vkPresentInfo_;
};

}  // namespace hk

#endif  // !HERAKLES_HERAKLES_VULKAN_SWAPCHAIN_HPP
