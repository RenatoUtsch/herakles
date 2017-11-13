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

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "herakles/scene/bvh.hpp"
#include "herakles/scene/camera.hpp"
#include "herakles/scene/scene_generated.h"
#include "herakles/vulkan/allocator.hpp"
#include "herakles/vulkan/buffer.hpp"
#include "herakles/vulkan/descriptor_pool.hpp"
#include "herakles/vulkan/descriptor_set.hpp"
#include "herakles/vulkan/descriptor_set_layout.hpp"
#include "herakles/vulkan/device.hpp"
#include "herakles/vulkan/image.hpp"
#include "herakles/vulkan/instance.hpp"
#include "herakles/vulkan/physical_device.hpp"
#include "herakles/vulkan/pipeline.hpp"
#include "herakles/vulkan/shader.hpp"
#include "herakles/vulkan/surface.hpp"
#include "herakles/vulkan/surface_provider.hpp"
#include "herakles/vulkan/swapchain.hpp"

// TODO(renatoutsch): validate these flags.
DEFINE_string(output_file, "",
              "Output file of the rendered surface. Will replace any "
              "existing files.");
DEFINE_string(
    surface_type, "windowed",
    "Surface type. One of \"windowed\",\"fullscreen\" and \"headless\".");
DEFINE_string(scene_file, "", "Binary .hks scene file to be rendered.");
DEFINE_string(shader_file, "", "Shader binary to be executed.");
DEFINE_string(shader_entry_point, "main", "Entry point of the shader binary.");
DEFINE_int32(width, 800, "Width resolution of the surface.");
DEFINE_int32(height, 600, "Height resolution of the surface.");
DEFINE_bool(enable_validation_layers, false,
            "If is to enable validation layers when running the program.");
DEFINE_bool(unlock_camera, false,
            "If is to unlock the camera and allow movement.");

namespace {
const char *RendererName = "Herakles Renderer";
const uint32_t RendererVersion = VK_MAKE_VERSION(0, 0, 0);

struct UniformBufferObject {
  hk::PinholeCamera camera;
  uint32_t frameCount = 0;

  UniformBufferObject(hk::PinholeCamera &&camera) : camera(camera) {}
};

std::vector<uint8_t> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  CHECK(file.is_open()) << "Couldn't open input scene file.";

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);
  CHECK(file.read((char *)buffer.data(), size));

  return buffer;
}

class Renderer {
 public:
  Renderer(const char *appName, uint32_t appVersion,
           const std::string &sceneFilename, const std::string &shaderFilename,
           const std::string shaderEntryPoint, int width, int height,
           bool fullscreen, bool enableValidationLayers)
      : sceneBuffer_(readFile(sceneFilename)),
        scene_(hk::scene::GetScene(sceneBuffer_.data())),
        instance_(appName, appVersion, enableValidationLayers,
                  surfaceProvider_),
        surface_(surfaceProvider_, instance_, appName, width, height,
                 fullscreen),
        pipeline_(device_,
                  hk::Shader(shaderFilename, shaderEntryPoint, device_),
                  descriptorSetLayout_),
        ubo_(scene_->camera()) {
    logSceneStats_();
    initializeGPUData_();
    LOG(INFO) << "Renderer initialized";
  }

  void run() {
    while (!surface_.programShouldExit()) {
      surface_.pollEvents();

      updateDeltaTime_();
      updateCamera_();
      updateFPS_();
      updateUBO_();
      drawFrame_();
    }

    device_.vkComputeQueue().waitIdle();
  }

 private:
  void updateDeltaTime_() {
    static auto lastTime = timer_.now();

    auto currentTime = timer_.now();
    deltaTime_ = std::chrono::duration_cast<std::chrono::duration<float>>(
                     currentTime - lastTime)
                     .count();

    lastTime = currentTime;
  }

  void updateCamera_() {
    if (FLAGS_unlock_camera && cameraManager_.update(ubo_.camera, deltaTime_)) {
      ubo_.frameCount = 0;  // Camera changed, reset frame count.
    }
  }

  void updateFPS_() {
    static float totalDelta = 0.0f;
    static int nFrames = 0;

    ++nFrames;
    totalDelta += deltaTime_;
    if (totalDelta >= 1.0f) {
      std::cout << nFrames << " FPS | " << 1000.0 / (double)nFrames
                << "ms/frame" << std::endl;
      nFrames = 0;
      totalDelta = 0.0f;
    }
  }

  void updateUBO_() {
    const auto &computeQueue = device_.vkComputeQueue();

    uboStagingBuffer_.mapMemory(
        [this](void *data) { memcpy(data, &ubo_, sizeof(ubo_)); });
    ++ubo_.frameCount;

    computeQueue.waitIdle();
    device_.submitOneTimeComputeCommands(
        [this](const vk::CommandBuffer &commandBuffer) {
          uboStagingBuffer_.copyTo(commandBuffer, uboBuffer_);
        });
    computeQueue.waitIdle();
  }

  void drawFrame_() {
    const auto &computeQueue = device_.vkComputeQueue();
    const auto result =
        swapchain_.acquireNextImage(0, *imageAvailableSemaphore_);
    if (result.result == vk::Result::eNotReady) {
      LOG(ERROR) << "Not ready";
      return;
    }
    if (result.result == vk::Result::eTimeout) {
      LOG(ERROR) << "Timeout";
      return;
    }

    const auto &imageIndex = result.value;

    computeQueue.submit(1, &swapchainSubmitInfos_[imageIndex], nullptr);

    swapchain_.presentImage(imageIndex, 1, &*renderFinishedSemaphore_);
  }

  hk::DescriptorSetLayout createDescriptorSetLayout_() {
    const size_t numBindings = 12;
    std::vector<vk::DescriptorSetLayoutBinding> bindings(numBindings);
    bindings[0]
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eStorageImage)
        .setDescriptorCount(1);
    bindings[1]
        .setBinding(1)
        .setDescriptorType(vk::DescriptorType::eStorageImage)
        .setDescriptorCount(1);
    bindings[2]
        .setBinding(2)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1);

    for (size_t i = 3; i < numBindings; ++i) {
      bindings[i]
          .setBinding(i)
          .setDescriptorType(vk::DescriptorType::eStorageBuffer)
          .setDescriptorCount(1);
    }

    return hk::DescriptorSetLayout(device_, bindings);
  }

  /// Creates the command buffers used when rendering, one for each image in
  // the swapchain.
  std::vector<vk::CommandBuffer> createSwapchainCommandBuffers_() {
    std::vector<vk::CommandBuffer> commandBuffers =
        device_.allocateComputeCommandBuffers(swapchain_.numImages());

    for (uint32_t i = 0; i < swapchain_.numImages(); ++i) {
      const auto &commandBuffer = commandBuffers[i];
      const auto &image = swapchain_.image(i);

      commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

      commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
                                 pipeline_.vkPipeline());

      commandBuffer.bindDescriptorSets(
          vk::PipelineBindPoint::eCompute, pipeline_.vkPipelineLayout(), 0, 1,
          &frameDescriptorSet_.vkDescriptorSet(), 0, nullptr);

      frameImage_.layoutTransitionBarrier(
          commandBuffer, vk::ImageLayout::eTransferSrcOptimal,
          vk::ImageLayout::eGeneral, vk::AccessFlagBits::eTransferRead,
          vk::AccessFlagBits::eShaderWrite);

      commandBuffer.dispatch(ceil((float)swapchain_.width() / 32),
                             ceil((float)swapchain_.height() / 32), 1);

      frameImage_.layoutTransitionBarrier(
          commandBuffer, vk::ImageLayout::eGeneral,
          vk::ImageLayout::eTransferSrcOptimal,
          vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead);

      image.layoutTransitionBarrier(commandBuffer, vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal, {},
                                    vk::AccessFlagBits::eTransferWrite);

      frameImage_.copyTo(commandBuffer, image,
                         vk::ImageLayout::eTransferSrcOptimal,
                         vk::ImageLayout::eTransferDstOptimal);

      image.layoutTransitionBarrier(
          commandBuffer, vk::ImageLayout::eTransferDstOptimal,
          vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits::eTransferWrite,
          vk::AccessFlagBits::eMemoryRead);

      commandBuffer.end();
    }

    return commandBuffers;
  }

  std::vector<vk::SubmitInfo> createSwapchainSubmitInfos_() {
    std::vector<vk::SubmitInfo> submitInfos(swapchain_.numImages());
    for (uint32_t i = 0; i < swapchain_.numImages(); ++i) {
      submitInfos[i]
          .setWaitSemaphoreCount(1)
          .setPWaitSemaphores(&*imageAvailableSemaphore_)
          .setPWaitDstStageMask(&swapchainWaitStage_)
          .setCommandBufferCount(1)
          .setPCommandBuffers(&swapchainCommandBuffers_[i])
          .setSignalSemaphoreCount(1)
          .setPSignalSemaphores(&*renderFinishedSemaphore_);
    }

    return submitInfos;
  }

  hk::Image createFrameImage_() {
    auto image = hk::Image(
        device_, swapchain_.width(), swapchain_.height(),
        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
        swapchain_.surfaceFormat().format);

    device_.submitOneTimeComputeCommands(
        [&image](const vk::CommandBuffer &commandBuffer) {
          image.layoutTransitionBarrier(commandBuffer,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferSrcOptimal,
                                        {}, vk::AccessFlagBits::eTransferRead);
        });
    device_.vkComputeQueue().waitIdle();

    LOG(INFO) << "Created frame image";
    return image;
  }

  hk::Image createSeedImage_() {
    auto image = hk::Image(
        device_, swapchain_.width(), swapchain_.height(),
        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst,
        vk::Format::eR32G32Uint);

    device_.submitOneTimeComputeCommands(
        [&image](const vk::CommandBuffer &commandBuffer) {
          image.layoutTransitionBarrier(
              commandBuffer, vk::ImageLayout::eUndefined,
              vk::ImageLayout::eGeneral, {}, vk::AccessFlagBits::eShaderRead);
        });
    device_.vkComputeQueue().waitIdle();

    LOG(INFO) << "Created seed image";
    return image;
  }

  hk::Buffer createStorageBuffer_(vk::DeviceSize size) {
    return hk::Buffer(device_, size,
                      vk::BufferUsageFlagBits::eUniformBuffer |
                          vk::BufferUsageFlagBits::eTransferDst);
  }

  template <typename T>
  hk::Buffer createStorageBuffer_(const flatbuffers::Vector<const T *> *vec) {
    return createStorageBuffer_(vec->size() * sizeof(T));
  }

  template <typename T>
  hk::Buffer createStorageBuffer_(const flatbuffers::Vector<T> *vec) {
    return createStorageBuffer_(vec->size() * sizeof(T));
  }

  template <typename T>
  hk::Buffer createStorageBuffer_(const std::vector<T> &vec) {
    return createStorageBuffer_(vec.size() * sizeof(T));
  }

  /// Staging buffer for the given buffer.
  hk::Buffer createStagingBuffer_(const hk::Buffer &buffer) {
    return hk::Buffer(device_, buffer.requestedSize(),
                      vk::BufferUsageFlagBits::eTransferSrc);
  }

  hk::SharedDeviceMemory createLocalImageMemory_() {
    LOG(INFO) << "Allocating local image memory";
    return hk::allocateMemory(device_, vk::MemoryPropertyFlagBits::eDeviceLocal,
                              {frameImage_, seedImage_});
  }

  hk::SharedDeviceMemory createLocalBufferMemory_() {
    LOG(INFO) << "Allocating local buffer memory";
    return hk::allocateMemory(
        device_, vk::MemoryPropertyFlagBits::eDeviceLocal,
        {uboBuffer_, bvhNodeBuffer_, bvhTriangleBuffer_, areaLightBuffer_,
         meshBuffer_, materialBuffer_, indexBuffer_, vertexBuffer_,
         normalBuffer_, uvBuffer_});
  }

  hk::SharedDeviceMemory createStagingBufferMemory_() {
    LOG(INFO) << "Allocating staging buffer memory";
    return hk::allocateMemory(device_,
                              vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent,
                              {uboStagingBuffer_});
  }

  /// Creates a descriptor set for when the swapchain is not acquired.
  hk::DescriptorSet createFrameDescriptorSet_() {
    return hk::DescriptorSet(
        descriptorPool_,
        {
            vk::DescriptorImageInfo(vk::Sampler(), *frameImageView_,
                                    vk::ImageLayout::eGeneral),
            vk::DescriptorImageInfo(vk::Sampler(), *seedImageView_,
                                    vk::ImageLayout::eGeneral),
            vk::DescriptorBufferInfo(uboBuffer_.vkBuffer(), 0,
                                     uboBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(bvhNodeBuffer_.vkBuffer(), 0,
                                     bvhNodeBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(bvhTriangleBuffer_.vkBuffer(), 0,
                                     bvhTriangleBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(areaLightBuffer_.vkBuffer(), 0,
                                     areaLightBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(meshBuffer_.vkBuffer(), 0,
                                     meshBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(materialBuffer_.vkBuffer(), 0,
                                     materialBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(indexBuffer_.vkBuffer(), 0,
                                     indexBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(vertexBuffer_.vkBuffer(), 0,
                                     vertexBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(normalBuffer_.vkBuffer(), 0,
                                     normalBuffer_.requestedSize()),
            vk::DescriptorBufferInfo(uvBuffer_.vkBuffer(), 0,
                                     uvBuffer_.requestedSize()),
        });
  }

  void logSceneStats_() {
    LOG(INFO) << "bvhNodes_.size(): " << bvhData_.nodes.size() << " ("
              << bvhNodeBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "bvhTriangles_.size(): " << bvhData_.triangles.size() << " ("
              << bvhTriangleBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "areaLights()->size(): " << scene_->areaLights()->size()
              << " (" << areaLightBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "meshes()->size(): " << scene_->meshes()->size() << " ("
              << meshBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "materials()->size(): " << scene_->materials()->size() << " ("
              << materialBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "indices()->size(): " << scene_->indices()->size() << " ("
              << indexBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "vertices()->size(): " << scene_->vertices()->size() << " ("
              << vertexBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "normals()->size(): " << scene_->normals()->size() << " ("
              << normalBuffer_.requestedSize() << " bytes)";
    LOG(INFO) << "uvs()->size(): " << scene_->uvs()->size() << " ("
              << uvBuffer_.requestedSize() << " bytes)";
  }

  /// Sets up a buffer with the given data accessor.
  void setupBuffer_(const hk::Buffer &buffer,
                    std::function<void *()> accessor) {
    hk::oneTimeSetup(buffer, [&](const hk::Buffer &stagingBuffer) {
      stagingBuffer.mapMemory([&](void *data) {
        memcpy(data, accessor(), stagingBuffer.requestedSize());
      });

      device_.submitOneTimeComputeCommands(
          [&](const vk::CommandBuffer &commandBuffer) {
            stagingBuffer.copyTo(commandBuffer, buffer);
          });
      device_.vkComputeQueue().waitIdle();
    });
  }

  /// Initializes the GPU data that doesn't need a persistent staging buffer.
  void initializeGPUData_() {
    hk::oneTimeSetup(seedImage_, [this](const hk::Buffer &stagingBuffer) {
      initializeSeeds_(stagingBuffer);
    });
    setupBuffer_(bvhNodeBuffer_,
                 [&]() { return (void *)bvhData_.nodes.data(); });
    setupBuffer_(bvhTriangleBuffer_,
                 [&]() { return (void *)bvhData_.triangles.data(); });
    if (areaLightBuffer_.requestedSize() > 0) {
      setupBuffer_(areaLightBuffer_,
                   [&]() { return (void *)scene_->areaLights()->Data(); });
    }
    setupBuffer_(meshBuffer_,
                 [&]() { return (void *)scene_->meshes()->Data(); });
    setupBuffer_(materialBuffer_,
                 [&]() { return (void *)scene_->materials()->Data(); });
    setupBuffer_(indexBuffer_,
                 [&]() { return (void *)scene_->indices()->Data(); });
    setupBuffer_(vertexBuffer_,
                 [&]() { return (void *)scene_->vertices()->Data(); });
    setupBuffer_(normalBuffer_,
                 [&]() { return (void *)scene_->normals()->Data(); });
    if (uvBuffer_.requestedSize() > 0) {
      setupBuffer_(uvBuffer_, [&]() { return (void *)scene_->uvs()->Data(); });
    }
  }

  /// Initializes the seeds used in rendering.
  void initializeSeeds_(const hk::Buffer &stagingBuffer) {
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());
    std::uniform_int_distribution<uint64_t> distribution;
    std::vector<uint64_t> randomNumbers(swapchain_.width() *
                                        swapchain_.height());

    for (auto &n : randomNumbers) {
      n = distribution(rng);
    }

    stagingBuffer.mapMemory([&randomNumbers](void *data) {
      memcpy(data, randomNumbers.data(),
             sizeof(randomNumbers[0]) * randomNumbers.size());
    });

    device_.submitOneTimeComputeCommands(
        [&](const vk::CommandBuffer &commandBuffer) {
          seedImage_.layoutTransitionBarrier(
              commandBuffer, vk::ImageLayout::eUndefined,
              vk::ImageLayout::eTransferDstOptimal, {},
              vk::AccessFlagBits::eTransferWrite);

          stagingBuffer.copyTo(commandBuffer, seedImage_);

          seedImage_.layoutTransitionBarrier(
              commandBuffer, vk::ImageLayout::eTransferDstOptimal,
              vk::ImageLayout::eGeneral, vk::AccessFlagBits::eTransferWrite,
              vk::AccessFlagBits::eShaderRead);
        });
    device_.vkComputeQueue().waitIdle();
  }

  const std::vector<uint8_t> sceneBuffer_;
  const hk::scene::Scene *scene_;
  hk::BVHData bvhData_ = hk::buildBVH(scene_);

  hk::SurfaceProvider surfaceProvider_;
  hk::Instance instance_;
  hk::Surface surface_;
  hk::PhysicalDevice physicalDevice_ =
      hk::pickPhysicalDevice(instance_, surface_);
  hk::Device device_ = hk::Device(instance_, physicalDevice_);
  hk::Swapchain swapchain_ = hk::Swapchain(surface_, device_);

  hk::DescriptorSetLayout descriptorSetLayout_ = createDescriptorSetLayout_();
  hk::Pipeline pipeline_;
  hk::DescriptorPool descriptorPool_ =
      hk::DescriptorPool(descriptorSetLayout_, 1);

  hk::Image frameImage_ = createFrameImage_();
  hk::Image seedImage_ = createSeedImage_();

  UniformBufferObject ubo_;
  hk::Buffer uboBuffer_ = createStorageBuffer_(sizeof(ubo_));
  hk::Buffer uboStagingBuffer_ = createStagingBuffer_(uboBuffer_);
  hk::Buffer bvhNodeBuffer_ = createStorageBuffer_(bvhData_.nodes);
  hk::Buffer bvhTriangleBuffer_ = createStorageBuffer_(bvhData_.triangles);
  hk::Buffer areaLightBuffer_ = createStorageBuffer_(scene_->areaLights());
  hk::Buffer meshBuffer_ = createStorageBuffer_(scene_->meshes());
  hk::Buffer materialBuffer_ = createStorageBuffer_(scene_->materials());
  hk::Buffer indexBuffer_ = createStorageBuffer_(scene_->indices());
  hk::Buffer vertexBuffer_ = createStorageBuffer_(scene_->vertices());
  hk::Buffer normalBuffer_ = createStorageBuffer_(scene_->normals());
  hk::Buffer uvBuffer_ = createStorageBuffer_(scene_->uvs());

  hk::SharedDeviceMemory localImageMemory_ = createLocalImageMemory_();
  hk::SharedDeviceMemory localBufferMemory_ = createLocalBufferMemory_();
  hk::SharedDeviceMemory stagingBufferMemory_ = createStagingBufferMemory_();

  vk::UniqueImageView frameImageView_ = frameImage_.createImageView();
  vk::UniqueImageView seedImageView_ = seedImage_.createImageView();

  hk::DescriptorSet frameDescriptorSet_ = createFrameDescriptorSet_();
  std::vector<vk::CommandBuffer> swapchainCommandBuffers_ =
      createSwapchainCommandBuffers_();
  std::vector<vk::SubmitInfo> swapchainSubmitInfos_ =
      createSwapchainSubmitInfos_();

  vk::UniqueSemaphore imageAvailableSemaphore_ = device_.createSemaphore();
  vk::UniqueSemaphore renderFinishedSemaphore_ = device_.createSemaphore();

  hk::CameraManager cameraManager_ = hk::CameraManager(surface_, ubo_.camera);

  std::chrono::high_resolution_clock timer_;
  float deltaTime_;  // In seconds

  const vk::PipelineStageFlags swapchainWaitStage_ =
      vk::PipelineStageFlagBits::eComputeShader;
};

}  // namespace

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  const std::string surfaceType = FLAGS_surface_type;
  bool fullscreen;
  if (surfaceType == "headless") {
    LOG(FATAL) << "surface_type headless is currently unsupported.";
  } else if (surfaceType == "windowed") {
    fullscreen = false;
  } else if (surfaceType == "fullscreen") {
    fullscreen = true;
  } else {
    LOG(FATAL) << "Invalid surface_type flag.";
  }

  Renderer renderer(RendererName, RendererVersion, FLAGS_scene_file,
                    FLAGS_shader_file, FLAGS_shader_entry_point, FLAGS_width,
                    FLAGS_height, fullscreen, FLAGS_enable_validation_layers);
  LOG(INFO) << "Created renderer";

  renderer.run();
  return 0;
}
