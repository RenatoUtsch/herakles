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

load("@com_github_renatoutsch_rules_spirv//glsl:defs.bzl", "glsl_library")

package(default_visibility = ["//visibility:public"])

glsl_library(
    name = "bsdf",
    srcs = ["bsdf.glsl"],
    deps = [
        ":random",
        ":scene",
    ],
)

glsl_library(
    name = "intersection",
    srcs = ["intersection.glsl"],
    deps = [
        ":random",
        ":scene",
    ],
)

glsl_library(
    name = "path_tracer",
    srcs = ["path_tracer.glsl"],
    deps = [
        ":bsdf",
        ":intersection",
        ":random",
        ":sampling",
        ":scene",
        ":utils",
    ],
)

glsl_library(
    name = "random",
    srcs = ["random.glsl"],
)

glsl_library(
    name = "sampling",
    srcs = ["sampling.glsl"],
    deps = [
        ":intersection",
        ":random",
        ":scene",
        ":utils",
    ],
)

glsl_library(
    name = "scene",
    srcs = ["scene.glsl"],
)

glsl_library(
    name = "utils",
    srcs = ["utils.glsl"],
)
