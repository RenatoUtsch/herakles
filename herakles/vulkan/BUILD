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

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "instance",
    srcs = ["instance.cpp"],
    hdrs = ["instance.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":surface_provider",
        ":utils",
        "//herakles/vulkan/internal:ext_loader",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "surface_provider",
    srcs = ["surface_provider.cpp"],
    hdrs = ["surface_provider.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        "//third_party:glfw",
        "//third_party:glog",
        "//third_party:vulkan",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "surface",
    srcs = ["surface.cpp"],
    hdrs = ["surface.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":instance",
        ":surface_provider",
        "//third_party:glfw",
        "//third_party:glog",
        "//third_party:vulkan",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "physical_device",
    srcs = ["physical_device.cpp"],
    hdrs = ["physical_device.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":instance",
        ":surface",
        ":surface_provider",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "device",
    srcs = ["device.cpp"],
    hdrs = ["device.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":instance",
        ":physical_device",
        ":utils",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "swapchain",
    srcs = ["swapchain.cpp"],
    hdrs = ["swapchain.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":device",
        ":image",
        ":surface",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "shader",
    srcs = ["shader.cpp"],
    hdrs = ["shader.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":device",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "pipeline",
    srcs = ["pipeline.cpp"],
    hdrs = ["pipeline.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":descriptor_set_layout",
        ":device",
        ":shader",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "descriptor_set_layout",
    srcs = ["descriptor_set_layout.cpp"],
    hdrs = ["descriptor_set_layout.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":device",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "descriptor_pool",
    srcs = ["descriptor_pool.cpp"],
    hdrs = ["descriptor_pool.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":descriptor_set_layout",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "descriptor_set",
    srcs = ["descriptor_set.cpp"],
    hdrs = ["descriptor_set.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":descriptor_pool",
        ":descriptor_set_layout",
        ":device",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "allocator",
    srcs = ["allocator.cpp"],
    hdrs = ["allocator.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":buffer",
        ":device",
        ":image",
        ":physical_device",
        "//third_party:glog",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "buffer",
    srcs = ["buffer.cpp"],
    hdrs = ["buffer.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":device",
        ":image",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "image",
    srcs = ["image.cpp"],
    hdrs = ["image.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":device",
        "//third_party:vulkan_hpp",
    ],
)

cc_library(
    name = "camera",
    srcs = ["camera.cpp"],
    hdrs = ["camera.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":surface",
        "//third_party:glm",
    ],
)

cc_library(
    name = "utils",
    srcs = ["utils.cpp"],
    hdrs = ["utils.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        "//third_party:glog",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":utils",
        "//third_party:gtest",
    ],
)
