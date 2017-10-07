# Copyright 2017 Renato Utsch
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("//herakles:defs.bzl", "HERAKLES_CPP_COPTS")

package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "renderer",
    srcs = [
        "renderer.cpp",
    ],
    copts = HERAKLES_CPP_COPTS,
    data = [
        "//renderer/shaders:main",
        "//renderer/shaders:red",
        "//renderer/shaders:smallpt",
    ],
    deps = [
        "//herakles/scene",
        "//herakles/vulkan:allocator",
        "//herakles/vulkan:buffer",
        "//herakles/vulkan:camera",
        "//herakles/vulkan:descriptor_pool",
        "//herakles/vulkan:descriptor_set",
        "//herakles/vulkan:descriptor_set_layout",
        "//herakles/vulkan:device",
        "//herakles/vulkan:image",
        "//herakles/vulkan:instance",
        "//herakles/vulkan:physical_device",
        "//herakles/vulkan:pipeline",
        "//herakles/vulkan:shader",
        "//herakles/vulkan:surface",
        "//herakles/vulkan:surface_provider",
        "//herakles/vulkan:swapchain",
        "//third_party:gflags",
        "//third_party:glm",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)
