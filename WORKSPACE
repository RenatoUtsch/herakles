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

workspace(name = "com_github_renatoutsch_herakles")

load("//herakles:defs.bzl", "herakles_repositories")

herakles_repositories()

load(
    "@com_github_renatoutsch_rules_spirv//spirv:defs.bzl",
    "spirv_repositories",
)

spirv_repositories(omit_com_github_renatoutsch_rules_system = True)

load(
    "@com_github_renatoutsch_rules_system//system:defs.bzl",
    "local_cc_library",
)

local_cc_library(
    name = "glfw",
    hdrs = [
        "GLFW/glfw3.h",
        "GLFW/glfw3native.h",
    ],
    libs = {
        "windows": ["glfw3.dll"],
        "macos": ["libglfw3.so"],
        "default": ["libglfw.so"],
    },
)

local_cc_library(
    name = "vulkan",
    # TODO(renatoutsch): add once environment variables are supported.
    #include_paths = ["${VULKAN_SDK}/Include"],
    #library_paths = ["${VULKAN_SDK}/Lib"],
    libs = {
        "windows": ["Vulkan-1.dll"],
        "default": ["libvulkan.so"],
    },
)

# TODO(renatoutsch): Replace this with proper vendoring.
local_cc_library(
    name = "glog",
    hdrs = [
        "glog/log_severity.h",
        "glog/logging.h",
        "glog/raw_logging.h",
        "glog/stl_logging.h",
        "glog/vlog_is_on.h",
    ],
    libs = {
        "windows": ["glog.dll"],
        "default": ["libglog.so"],
    },
)
