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
load(
    "@com_github_renatoutsch_rules_flatbuffers//flatbuffers:defs.bzl",
    "cc_flatbuffer_library",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "bounds",
    hdrs = ["bounds.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        "//third_party:glm",
    ],
)

cc_library(
    name = "bvh",
    srcs = ["bvh.cpp"],
    hdrs = ["bvh.hpp"],
    deps = [
        ":bounds",
        ":scene",
        "//third_party:glm",
    ],
)

cc_library(
    name = "camera",
    srcs = ["camera.cpp"],
    hdrs = ["camera.hpp"],
    copts = HERAKLES_CPP_COPTS,
    deps = [
        ":scene",
        "//herakles/vulkan:surface",
        "//third_party:glm",
    ],
)

cc_flatbuffer_library(
    name = "scene",
    srcs = ["scene.fbs"],
)
