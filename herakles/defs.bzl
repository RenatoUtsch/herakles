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

# Use this in all C++ cc_library and cc_binary rules that are part of or depend
# on Herakles.
HERAKLES_CPP_COPTS = [
    "-Wall",
    "-Wextra",
    "-Werror",
    "-pedantic",
    "-std=c++1z",
]

# Use this in all C cc_library and cc_binary rules that are part of or depend
# on Herakles.
HERAKLES_C_COPTS = [
    "-Wall",
    "-Wextra",
    "-Werror",
    "-pedantic",
    "-std=c11",
]

def herakles_repositories(omit_com_github_renatoutsch_rules_flatbuffers=False,
                          omit_com_github_renatoutsch_rules_system=False,
                          omit_com_github_renatoutsch_rules_spirv=False,
                          omit_com_github_gflags_gflags=False,
                          omit_com_github_gtruc_glm=False,
                          omit_com_github_khronosgroup_vulkan_docs=False,
                          omit_com_github_khronosgroup_vulkan_hpp=False,
                          omit_com_github_nothings_stb=False,
                          omit_com_google_glog=False,
                          omit_com_google_googletest=False):
    """Imports dependencies for Herakles."""
    if not omit_com_github_renatoutsch_rules_flatbuffers:
        com_github_renatoutsch_rules_flatbuffers()
    if not omit_com_github_renatoutsch_rules_system:
        com_github_renatoutsch_rules_system()
    if not omit_com_github_renatoutsch_rules_spirv:
        com_github_renatoutsch_rules_spirv()
    if not omit_com_github_gflags_gflags:
        com_github_gflags_gflags()
    if not omit_com_github_gtruc_glm:
        com_github_gtruc_glm()
    if not omit_com_github_khronosgroup_vulkan_docs:
        com_github_khronosgroup_vulkan_docs()
    if not omit_com_github_khronosgroup_vulkan_hpp:
        com_github_khronosgroup_vulkan_hpp()
    if not omit_com_google_glog:
        com_google_glog()
    if not omit_com_google_googletest:
        com_google_googletest()
    if not omit_com_github_nothings_stb:
        com_github_nothings_stb()

def com_github_renatoutsch_rules_flatbuffers():
    # TODO(renatoutsch): use a tag or commit once there's a release
    native.http_archive(
        name = "com_github_renatoutsch_rules_flatbuffers",
        #sha256 = "",  # TODO(renatoutsch): add once there's a release
        strip_prefix = "rules_flatbuffers-master",
        urls = ["https://github.com/RenatoUtsch/rules_flatbuffers/archive/master.zip"],
    )

def com_github_renatoutsch_rules_system():
    # TODO(renatoutsch): use a tag or commit once there's a release
    native.http_archive(
        name = "com_github_renatoutsch_rules_system",
        #sha256 = "",  # TODO(renatoutsch): add once there's a release
        strip_prefix = "rules_system-master",
        urls = ["https://github.com/RenatoUtsch/rules_system/archive/master.zip"],
    )

def com_github_renatoutsch_rules_spirv():
    # TODO(renatoutsch): use a tag or commit once there's a release
    native.http_archive(
        name = "com_github_renatoutsch_rules_spirv",
        #sha256 = "",  # TODO(renatoutsch): add once there's a release
        strip_prefix = "rules_spirv-master",
        urls = ["https://github.com/RenatoUtsch/rules_spirv/archive/master.zip"],
    )

def com_github_gflags_gflags():
    native.http_archive(
        name = "com_github_gflags_gflags",
        sha256 = "4e44b69e709c826734dbbbd5208f61888a2faf63f239d73d8ba0011b2dccc97a",
        strip_prefix = "gflags-2.2.1",
        urls = ["https://github.com/gflags/gflags/archive/v2.2.1.zip"],
    )

def com_github_gtruc_glm():
    native.new_http_archive(
        name = "com_github_gtruc_glm",
        build_file = "third_party/glm.BUILD",
        sha256 = "0b4c56d74618235ffe8d92f44ec7daef9506923c51762546df7ea4fc8e21face",
        strip_prefix = "glm-0.9.8.5",
        urls = ["https://github.com/g-truc/glm/archive/0.9.8.5.zip"],
    )

def com_github_khronosgroup_vulkan_docs():
    native.new_http_archive(
        name = "com_github_khronosgroup_vulkan_docs",
        build_file = "third_party/vulkan_docs.BUILD",
        sha256 = "9d0376c5df89e8a5ac7f945754f6135f9c779bdb38a1d99b416f2a2943e2c118",
        strip_prefix = "Vulkan-Docs-1.0.57-core",
        urls = ["https://github.com/KhronosGroup/Vulkan-Docs/archive/v1.0.57-core.zip"],
    )

def com_github_khronosgroup_vulkan_hpp():
    native.new_http_archive(
        name = "com_github_khronosgroup_vulkan_hpp",
        build_file = "third_party/vulkan_hpp.BUILD",
        sha256 = "719503f0dc4ea8b091aeb8aaaf487965d5dc8adea75c673861fd4d988b326391",
        strip_prefix = "Vulkan-Hpp-bca6564dac806ea8d30bad792066a3ba963fdbf1",
        urls = ["https://github.com/KhronosGroup/Vulkan-Hpp/archive/bca6564dac806ea8d30bad792066a3ba963fdbf1.zip"],
    )

def com_github_nothings_stb():
    native.new_http_archive(
        name = "com_github_nothings_stb",
        build_file = "third_party/stb/stb.BUILD",
        sha256 = "33e55ecfea2a78516a4fe42b92580ed45caceda26bdee3ac6c933c677c0965fa",
        strip_prefix = "stb-9d9f75eb682dd98b34de08bb5c489c6c561c9fa6",
        urls = ["https://github.com/nothings/stb/archive/9d9f75eb682dd98b34de08bb5c489c6c561c9fa6.zip"],
    )

def com_google_glog():
    native.new_http_archive(
        name = "com_google_glog",
        build_file = "third_party/glog.BUILD",
        sha256 = "267103f8a1e9578978aa1dc256001e6529ef593e5aea38193d31c2872ee025e8",
        strip_prefix = "glog-0.3.5",
        urls = ["https://github.com/google/glog/archive/v0.3.5.zip"],
    )

def com_google_googletest():
    native.new_http_archive(
        name = "com_google_googletest",
        build_file = "third_party/googletest.BUILD",
        sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
        strip_prefix = "googletest-release-1.8.0",
        urls = ["https://github.com/google/googletest/archive/release-1.8.0.zip"],
    )
