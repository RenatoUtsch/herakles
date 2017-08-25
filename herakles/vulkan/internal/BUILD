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

load("//herakles:defs.bzl", "HERAKLES_C_COPTS")

package(default_visibility = ["//herakles:__subpackages__"])

cc_library(
    name = "ext_loader",
    srcs = ["ext_loader.c"],
    hdrs = ["ext_loader.h"],
    copts = HERAKLES_C_COPTS,
    deps = [
        "@vulkan_docs//:vulkan",
    ],
)
